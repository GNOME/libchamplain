#!/usr/bin/perl

=head1 NAME

flickr.pl - Display geo tagged photos from Flickr

=head1 SYNOPSIS

flickr.pl key

Where I<key> is a valid Flickr key.

=head1 DESCRIPTION

This sample scripts shows how to interact with the Flickr API and to display
thumbnails for pictures near a location. The Flickr API interaction is triggered
when a middle-click in done in a location  on the map.

=cut

use strict;
use warnings;
use open ':std', ':utf8';

use Glib qw(TRUE FALSE);
use Clutter qw(-gtk-init);
use Gtk2 qw(-init);
use Champlain;
use LWP::UserAgent;
use XML::LibXML;
use Carp;
use URI;
use URI::QueryParam;
use Data::Dumper;

exit main();

sub main {

	die "Usage: flickr-key\n" unless @ARGV;
	my ($key) = @ARGV;

	local $| = 1;

	my $window = Gtk2::Window->new();
	$window->set_border_width(10);
	$window->set_title("Champlain + Flickr - Demo");
	$window->signal_connect('destroy' => sub { Gtk2->main_quit() });

	my $vbox = Gtk2::VBox->new(FALSE, 10);

	# Create the map view
	my $gtk2_map = Gtk2::ChamplainEmbed->new();
	my $map = $gtk2_map->get_view();
	$map->center_on(47.130885, -70.764141);
	$map->set_scroll_mode('kinetic');
	$map->set_zoom_level(5);
	$gtk2_map->set_size_request(640, 480);

	# Create the markers and marker layer
	my $layer = Champlain::Layer->new();
	$layer->show();
	$map->add_layer($layer);

	my $viewport = Gtk2::Viewport->new();
	$viewport->set_shadow_type('etched-in');
	$viewport->add($gtk2_map);

	$vbox->add($viewport);

	$window->add($vbox);
	$window->show_all();

	# Middle click on a location to trigger the Flickr interaction
	$map->set_reactive(TRUE);
	my $data = {
		layer => $layer,
		soup  => My::Soup->new('http://www.flickr.com', $key),
	};
	$map->signal_connect_after("button-release-event", \&flickr_search, $data);

	Gtk2->main();

	return 0;
}


#
# This callback starts the search for flickr pictures
#
sub flickr_search {
	my ($map, $event, $data) = @_;
	return FALSE unless $event->button == 2 && $event->click_count == 1;

	my ($latitude, $longitude) = $map->get_coords_from_event($event);

	my $args = {
		lat    => $latitude,
		lon    => $longitude,
		extras => 'geo',  # Return the location of the picture
	};

	$data->{soup}->do_flickr_request(
		'flickr.photos.search' => $args,
		\&flickr_photos_search_callback, $data,
	);

	return TRUE;
}


sub flickr_photos_search_callback {
	my ($soup, $uri, $response, $data) = @_;
	my %data = %{ $data };

	my $xml = $response->decoded_content;
	my $parser = XML::LibXML->new();
	my $doc = $parser->parse_string($xml);


	my @nodes = $doc->findnodes('/rsp/photos/photo[position() <= 5]');
	my @photos = ();
	foreach my $photo_node (@nodes) {
		my $id = $photo_node->getAttribute('id');
		my $secret = $photo_node->getAttribute('secret');
		my $latitude = $photo_node->getAttribute('latitude');
		my $longitude = $photo_node->getAttribute('longitude');
		my $accuracy = $photo_node->getAttribute('accuracy'); # In which zoom level was the photo tagged
		my $photo = {
			id        => $id,
			secret    => $secret,
			latitude  => $latitude,
			longitude => $longitude,
			accuracy  => $accuracy,
		};
		push @photos, $photo;
	}

	$data{photos} = \@photos;
	flickr_photos_getSizes($soup, \%data);
}


sub flickr_photos_getSizes {
	my ($soup, $data) = @_;
	if (@{ $data->{photos} } == 0) {
		return FALSE;
	}
	my %data = %{ $data };

	my $photo = pop @{ $data{photos} };
	$data{photo} = $photo;

	my $args = {
		photo_id => $photo->{id},
	};
	$soup->do_flickr_request(
		'flickr.photos.getSizes' => $args,
		\&flickr_photos_getSizes_callback, \%data,
	);

	return TRUE;
}


sub flickr_photos_getSizes_callback {
	my ($soup, $uri, $response, $data) = @_;
	my $xml = $response->decoded_content;
	my $parser = XML::LibXML->new();
	my $doc = $parser->parse_string($xml);

	# Display only the thumbnails ("Square" images)
	my ($node) = $doc->findnodes('/rsp/sizes/size[@label = "Square"]');
	if ($node) {
		my $url = $node->getAttribute('source');

		my $latitude  = $data->{photo}{latitude};
		my $longitude = $data->{photo}{longitude};
		my $uri = $node->getAttribute('source');

		# The image download is made from a different server than the RPC calls
		my $static_soup = My::Soup->new($uri);
		$static_soup->do_get(
			$uri,
			\&flickr_download_photo_callback,
			{
				latitude  => $latitude,
				longitude => $longitude,
				layer     => $data->{layer},
			},
		);
	}

	# Go on to the next photo
	flickr_photos_getSizes($soup, $data);
}



sub flickr_download_photo_callback {
	my ($self, $uri, $response, $data) = @_;

	if (! $response->is_success) {
		warn $response->status_line;
		return;
	}

	# Load the image with a Pixbuf Loader
	my $mime = $response->header('content-type');
	my $loader = Gtk2::Gdk::PixbufLoader->new_with_mime_type($mime);
	$loader->write($response->content);
	$loader->close;
	my $pixbuf = $loader->get_pixbuf;

	# Transform the Pixbuf into a Clutter::Texture
	my $texture = Clutter::Texture->new();
	$texture->set_from_rgb_data(
		$pixbuf->get_pixels,
		$pixbuf->get_has_alpha,
		$pixbuf->get_width,
		$pixbuf->get_height,
		$pixbuf->get_rowstride,
		($pixbuf->get_has_alpha ? 4 : 3),
		[]
	);

	# Add a marker for the image
	my $marker = Champlain::Marker->new_with_image($texture);
	$marker->set_position($data->{latitude}, $data->{longitude});
	$data->{layer}->add($marker);
	$marker->show();
}




#
# A very cheap implementation of an asynchronous HTTP client that integrates
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
	my ($uri, $key) = @_;

	my $self = bless {}, ref $class || $class;

	$uri = to_uri($uri);
	$self->{port} = $uri->port;
	$self->{host} = $uri->host;
	$self->{key} = $key;

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


sub do_flickr_request {
	my $self = shift;
	my ($method, $args, $callback, $data) = @_;


	# Construct the flickr request
	my $uri = URI->new('http://www.flickr.com/services/rest/');
	$uri->query_form_hash($args);
	$uri->query_param(method => $method);
	$uri->query_param(api_key => $self->{key});

	$self->do_get($uri, $callback, $data);
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
