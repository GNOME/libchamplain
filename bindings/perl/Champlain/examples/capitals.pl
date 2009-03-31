#!/usr/bin/perl

=head1 NAME

capitals.pl - Show the capitals

=head1 DESCRIPTION

This program takes you into a magical trip and displays the capital cities one
by one.

=cut

use strict;
use warnings;
use open ':std', ':utf8';

use Glib qw(TRUE FALSE);
use Clutter qw(-gtk-init);
use Gtk2 qw(-init);
use Champlain;

use XML::LibXML;


exit main();


sub main {

	my $window = Gtk2::Window->new();
	my $vbox = Gtk2::VBox->new(FALSE, 0);
	
	
	# Create the map stuff
	my $map = Champlain::View->new();
	my $gtk2_map = Gtk2::Champlain::ViewEmbed->new($map);
	$gtk2_map->set_size_request(640, 480);
	$map->center_on(0, 0);
	$map->set('scroll-mode', 'kinetic', 'zoom-level', 3);
	
	my $layer = Champlain::Layer->new();
	$map->add_layer($layer);
	
	
	my $viewport = Gtk2::Viewport->new();
	$viewport->set_shadow_type('etched-in');
	$viewport->add($gtk2_map);
	$vbox->pack_start($viewport, TRUE, TRUE, 0);
	
	$window->add($vbox);
	$window->set_size_request($gtk2_map->get_size_request);
	$window->signal_connect(destroy => sub {
		Gtk2->main_quit();
	});
	$window->show_all();

	
	my $capitals_url = "http://en.wikipedia.org/wiki/List_of_national_capitals";
	my $soup = My::Soup->new($capitals_url);
	
	$soup->do_get(
		$capitals_url,
		\&capitals_main_callback,
		{
			map   => $map,
			layer => $layer,
			markers => [],
		}
	);
	
	
	Gtk2->main();
	
	
	return 0;
}


#
# Called when the main page with all the capitals is downloaded.
#
sub capitals_main_callback {
	my ($soup, $uri, $response, $data) = @_;
	
	my $parser = XML::LibXML->new();
	$parser->recover_silently(1);
	$data->{parser} = $parser;

	# Find the table with the capitals
	my $document = $parser->parse_html_string($response->content);
	my @nodes = $document->findnodes('//table[@class="wikitable sortable"]/tr/td[1]/a');
	
	# Get the capitals
	my @capitals = ();
	foreach my $node (@nodes) {
		my $uri = $node->getAttribute('href') or next;
		my $name = $node->getAttribute('title') or next;
		my $capitals = {
			uri => $uri,
			name => $name,
		};
		push @capitals, $capitals;
	}
	
	# Download the capitals (the download is node one capital at a time)
	$data->{capitals} = \@capitals;
	download_capital($soup, $data);
}


#
# Called when the page of a capital is downloaded. The page is expected to have
# the coordinates of the capital.
#
sub capital_callback {
	my ($soup, $uri, $response, $data) = @_;
	
	my $document = $data->{parser}->parse_html_string($response->content);
	my $heading = $document->getElementById('firstHeading');
	if ($heading) {
		my $name = $document->getElementById('firstHeading')->textContent;
				
		my ($geo) = $document->findnodes('id("coordinates")//span[@class="geo"]');
		if ($geo) {
			my ($latitude, $longitude) = split /\s*;\s*/, $geo->textContent;
			$data->{current}{latitude} = $latitude;
			$data->{current}{longitude} = $longitude;
			printf "$name %.4f, %.4f\n", $latitude, $longitude;
	
			my $marker = Champlain::Marker->new_with_label($name, "Sans 15", undef, undef);
			$marker->set_position($latitude, $longitude);
			if (@{ $data->{markers} } == 5) {
				my $old = shift @{ $data->{markers} };
				$data->{layer}->remove($old);
			}
			push @{ $data->{markers} }, $marker;
			$data->{layer}->add($marker);
			$data->{map}->go_to($latitude, $longitude);
		}
	}
	
	Glib::Timeout->add (1_000, sub {
		download_capital($soup, $data);
		return FALSE;
	});
}


#
# This function downloads the page of a capital and then call it self again with
# the next capital to download. This process is repeated until there are no more
# capitals to download.
#
# The capitals to download are taken from $data->{capitals}.
#
sub download_capital {
	my ($soup, $data) = @_;

	my $capital = shift @{ $data->{capitals} };
	if (! defined $capital) {
		print "No more capitals to download\n";
		return;
	}
	
	my $uri = $capital->{uri};
	my $name = $capital->{name};
	$data->{current} = $capital;
	$soup->do_get($uri, \&capital_callback, $data);
}



#
# A very cheap implementation of an asynchornous HTTP client that integrates
# with Glib's main loop. This client implements a rudimentary version of
# 'Keep-Alive'.
#
# Each instance of this class can only make HTTP GET requests and only to a
# single HTTP server.
#
#
# Usage:
#
#   my $soup = My::Soup->new('http://en.wikipedia.com/');
#   $soup->do_get('http://en.wikipedia.com/Bratislava', sub {
#     my ($soup, $uri, $response, $data) = @_;
#     print $response->content;
#   });
#
package My::Soup;

use Glib qw(TRUE FALSE);
use Net::HTTP::NB;
use HTTP::Response;
use URI;


sub new {
	my $class = shift;
	my ($uri) = @_;
	
	my $self = bless {}, ref $class || $class;

	$uri = to_uri($uri);
	$self->{port} = $uri->port;
	$self->{host} = $uri->host;
	
	$self->connect();
	
	return $self;
}


#
# Connects to the remote HTTP server.
#
sub connect {
	my $self = shift;
	my $http = Net::HTTP::NB->new(
		Host      => $self->{host},
		PeerPort  => $self->{port},
		KeepAlive => 1,
	);
	$self->http($http);
}


sub http {
	my $self = shift;
	if (@_) {
		$self->{http} = $_[0];
	}
	return $self->{http};
}


sub to_uri {
	my ($uri) = @_;
	return $uri if ref($uri) && $uri->isa('URI');
	return URI->new($uri);
}


#
# Performs an HTTP GET request asynchronously.
#
sub do_get {
	my $self = shift;
	my ($uri, $callback, $data) = @_;
	$uri = to_uri($uri);
	
	# Note that this is not asynchronous!
	$self->http->write_request(GET => $uri->path_query);
	
	
	my ($code, $message, %headers);
	my $content = "";
	Glib::IO->add_watch($self->http->fileno, ['in'], sub {
		my (undef, $condition) = @_;
		
		# Read the headers
		if (!$code) {
			eval {
				($code, $message, %headers) = $self->http->read_response_headers();
			};
			if (my $error = $@) {
				# The server closed the socket reconnect and resume the HTTP GET
				$self->connect();
				$self->do_get($uri, $callback, $data);
				
				# We abort this I/O watch since another download will be started
				return FALSE;
			}
			
			# We return and continue when the server will have more data
			return TRUE;
		}
		
		
		# Read the content		
		my $line;
		my $n = $self->http->read_entity_body($line, 1024);
		$content .= $line;
		
		if ($self->http->keep_alive) {
			# In the case where the HTTP request has keep-alive we need to see if the
			# content has all arrived as read_entity_body() will not tell when the end
			# of the content has been reached.
			return TRUE unless length($content) == $headers{'Content-Length'};
		}
		elsif ($n) {
			# There's still data to read
			return TRUE;
		}
		
		# End of the document
		my $response = HTTP::Response->new($code, $message, [%headers], $content);
		$callback->($self, $uri, $response, $data);
		return FALSE;
	});
}

# A true value
1;
