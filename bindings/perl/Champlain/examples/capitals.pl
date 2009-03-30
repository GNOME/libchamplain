#!/usr/bin/perl

use strict;
use warnings;
use open ':std', ':utf8';
 
use Net::HTTP::NB;
use IO::Select;
use Glib qw(TRUE FALSE);
use URI;
use Carp;
use XML::LibXML;
use Data::Dumper;

exit main();


sub main {

	my $loop = Glib::MainLoop->new();
	
	my $capitals_url = "http://en.wikipedia.org/wiki/List_of_national_capitals";
	my $soup = My::Soup->new($capitals_url);
	
	my $parser = XML::LibXML->new();
	$parser->recover_silently(1);

	$soup->do_get(
		$capitals_url,
		\&capitals_main_callback,
		{
			loop => $loop,
			parser => $parser,
		}
	);
	
	Glib::Idle->add(sub {
		$soup->do_get($capitals_url, sub {
			my ($soup, $uri, $response, $data) = @_;
			printf "Downloaded $uri %s - %s\n", $response->code, $response->message;
			
			$soup->do_get("/wiki/Madrid", sub {
				my ($soup, $uri, $response, $data) = @_;
				printf "Downloaded $uri %s - %s\n", $response->code, $response->message;
				
				my $document = $parser->parse_html_string($response->content);
				my $name = $document->getElementById('firstHeading')->textContent;
				my ($latitude, $longitude) = (0, 0);
				
				my ($geo) = $document->findnodes('id("coordinates")//span[@class="geo"]');
				if ($geo) {
#					my @nodes = $coord_node->findnodes('//span[@class="latitude"]');
#					print "nodes = @nodes\n";
#					print $coord_node->toString(2);
#					my $latitude = $coord_node->findvalue('//span[@class="latitude"]');
#					my $longitude = $coord_node->findvalue('//span[@class="longitude"]');
					
#					my ($geo) = $coord_node->findnodes('//span[@class="geo"]');
					
					($latitude, $longitude) = split /\s*;\s*/, $geo->textContent;
					
#					printf "latitude = --$latitude-- = %.4f\n", dms_to_decimal($latitude);
#					printf "longitude = $longitude = %.4f\n", dms_to_decimal($longitude);
				}
				
				printf "$name %.4f, %.4f\n", $latitude, $longitude;
				
				$loop->quit();
			});
			
		});
		return FALSE;
	}) if 0;
	
	$loop->run();
	
	return 0;
}


#
# Called when the main page with all the capitals is downloaded
#
sub capitals_main_callback {
	my ($soup, $uri, $response, $data) = @_;
	
	printf "Downloaded $uri %s - %s\n", $response->code, $response->message;
	
	my $document = $data->{parser}->parse_html_string($response->content);
	my @nodes = $document->findnodes('//table[@class="wikitable sortable"]/tr/td[1]/a');
	
	$data->{nodes} = \@nodes;
	print "Download the first capital\n";
	download_capital($soup, $data);
	
#	foreach my $node (@nodes) {
#		my $uri = $node->getAttribute('href');
#		my $name = $node->getAttribute('title');
#		print "$name -> $uri\n";
#	}
#	printf "Found %d\n", scalar @nodes;
	
#	$data->{loop}->quit();
}


#
# Called when the page of a capital is downloaded
#
sub capital_callback {
	my ($soup, $uri, $response, $data) = @_;
	
	printf "Downloaded $uri %s - %s\n", $response->code, $response->message;
	
	my $document = $data->{parser}->parse_html_string($response->content);
	my $heading = $document->getElementById('firstHeading');
	if ($heading) {
		my $name = $document->getElementById('firstHeading')->textContent;
		my ($latitude, $longitude) = (0, 0);
				
		my ($geo) = $document->findnodes('id("coordinates")//span[@class="geo"]');
		if ($geo) {
			($latitude, $longitude) = split /\s*;\s*/, $geo->textContent;
		}
		printf "$name %.4f, %.4f\n", $latitude, $longitude;
	}
	
	download_capital($soup, $data);
}


sub download_capital {
	my ($soup, $data) = @_;

	print "download_capital nodes data = $data $data->{nodes}\n";
	
	my $node = shift @{ $data->{nodes} };
	if (! defined $node) {
		print "No more capitals to download\n";
		$data->{loop}->quit();
		return;
	}
	
	my $uri = $node->getAttribute('href');
	my $name = $node->getAttribute('title');
	print "$name -> $uri\n";
	$soup->do_get($uri, \&capital_callback, $data);
}


sub dms_to_decimal {
	my ($dms) = @_;
	
	my $regexp = qr/
		(\d+)\x{b0}        # Degrees
		(\d+)\x{2032}      # Minutes
		(?:(\d+)\x{2032})? # Seconds
		([NSEW])           # Direction
	/x;
	my (@fields) = ($dms =~ /$regexp/);
	
	return 0 unless @fields;
	
	my ($degrees, $minutes, $seconds, $direction) = map { $_ || 0 } @fields;
	my $decimal = $degrees + ($minutes * 60 + $seconds)/3600;
	return ($direction eq 'N' or $direction eq 'E') ? $decimal : -$decimal;
}


package My::Soup;

use Glib qw(TRUE FALSE);
use Data::Dumper;
use HTTP::Response;


sub new {
	my $class = shift;
	my ($uri) = @_;
	
	my $self = bless {}, ref $class || $class;

	$uri = to_uri($uri);
	my $http = Net::HTTP::NB->new(
		Host      => $uri->host,
		PeerPort  => $uri->port,
		KeepAlive => 1,
	);
	$self->http($http);
	
	return $self;
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
	
	# Note this is not asynchronous
	$self->http->write_request(GET => $uri->path_query);
	
	
	my ($code, $message, %headers);
	my $content = "";
	Glib::IO->add_watch($self->http->fileno, 'in', sub {
		
		# Read the headers
		if (!$code) {
			($code, $message, %headers) = $self->http->read_response_headers();
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
