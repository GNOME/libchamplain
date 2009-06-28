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
use XML::LibXML;
use Carp;
use URI;
use URI::QueryParam;
use Data::Dumper;
use FindBin;
use File::Spec;

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

	my $icon = Clutter::Texture->new(
		File::Spec->catfile($FindBin::Bin, 'images', 'flickr.png')
	);

	# Middle click on a location to trigger the Flickr interaction
	$map->set_reactive(TRUE);
	my $data = {
		layer => $layer,
		soup  => My::Soup::Flickr->new($key),
		icon  => $icon,
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
	print "Lookup for ($latitude, $longitude)\n";

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


#
# This callback gets called when a flickr search returns results. The reponse is
# going to contain the id, latitude and longitude of the pictures close to the
# geographical location requested.
#
# Once the pictures information is available this callback will create a new
# marker with a pending image and will schedule a flickr query for each picture
# in order to have the real URI where the picture can be downloaded. Each
# picture has to be queried individually.
#
sub flickr_photos_search_callback {
	my ($soup, $xml, $headers, $data) = @_;
	my $parser = XML::LibXML->new();
	my $doc = $parser->parse_string($xml);


	my @nodes = $doc->findnodes('/rsp/photos/photo[position() < 10]');
	my @photos = ();
	foreach my $photo_node (@nodes) {
		my $id = $photo_node->getAttribute('id');
		my $latitude = $photo_node->getAttribute('latitude');
		my $longitude = $photo_node->getAttribute('longitude');

		# Add a marker for the image
		my $icon = Clutter::Texture::Clone->new($data->{icon});
		my $marker = Champlain::Marker->new_with_image($icon);
		$marker->set_position($latitude, $longitude);
		$data->{layer}->add($marker);
		$marker->show();

		my $photo = {
			id     => $id,
			marker => $marker,
		};

		push @photos, $photo;
	}

	$data->{photos} = \@photos;
	flickr_photos_getSizes($soup, $data);
}


#
# Request the "size" of a single flickr picture. The size is not soo important,
# what matters here is that the answer will return the URI of the picture.
#
sub flickr_photos_getSizes {
	my ($soup, $data) = @_;
	if (@{ $data->{photos} } == 0) {
		return FALSE;
	}

	my $photo = pop @{ $data->{photos} };
	$data->{photo} = $photo;

	my $args = {
		photo_id => $photo->{id},
	};
	$soup->do_flickr_request(
		'flickr.photos.getSizes' => $args,
		\&flickr_photos_getSizes_callback, $data,
	);

	return TRUE;
}


#
# This callback gets called each time that flikr answers to a 'size' query. Here
# the square size is the only one that gets inspected all other sizes are
# silently ignored.
#
# This function will trigger the download of the square image.
#
sub flickr_photos_getSizes_callback {
	my ($soup, $xml, $headers, $data) = @_;
	my $parser = XML::LibXML->new();
	my $doc = $parser->parse_string($xml);

	# Display only the thumbnails ("Square" images)
	my ($node) = $doc->findnodes('/rsp/sizes/size[@label = "Square"]');
	if ($node) {
		my $uri = $node->getAttribute('source');

		# The image download is made from a different server than the RPC calls
		$data->{soup}->do_get(
			$uri,
			\&flickr_download_photo_callback,
			$data->{photo}{marker},
		);
	}

	# Go on to the next photo
	flickr_photos_getSizes($soup, $data);
}


#
# This callback gets called each time that a flikr image is downloaded. Once a
# picture is successfully downloaded it will replace the one in the current
# marker.
#
sub flickr_download_photo_callback {
	my ($soup, $content, $headers, $marker) = @_;

	if ($headers->{Status} !~ /^2\d\d/) {
		warn "$headers->{Status} $headers->{Reason}";
		return;
	}

	# Load the image with a Pixbuf Loader
	my ($mime) = split(/\s*;/, $headers->{'content-type'}, 1);

	my $loader = Gtk2::Gdk::PixbufLoader->new_with_mime_type($mime);
	$loader->write($content);
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
	$marker->set_image($texture);
}




#
# A very simple implementation of an asynchronous HTTP client that integrates
# with Glib's main loop.
#
# Usage:
#
#   my $soup = My::Soup::Flickr->new($key); # The key is for web service calls
#   $soup->do_flickr_request(
#     'flickr.photos.getSizes' => {photo_id => $id},
#     \&flickr_photos_getSizes_callback, $data,
#   });
#
package My::Soup::Flickr;

use Glib qw(TRUE FALSE);
use AnyEvent::HTTP;
use URI;


sub new {
	my $class = shift;
	my ($key) = @_;

	my $self = bless {}, ref $class || $class;
	$self->{key} = $key;

	return $self;
}


#
# Calls a Flickr web service asynchronously.
#
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
	$uri = URI->new($uri) unless ref($uri) && $uri->isa('URI');

	# Note that this is not asynchronous!
	print $uri, "\n";
	http_request(
		GET     => $uri,
		timeout => 10,
		sub {
			my ($content, $headers) = @_;
			$callback->($self, $content, $headers, $data);
		}
	);

	return FALSE;
}

# A true value
1;
