#!/usr/bin/perl

=head1 NAME

image.pl - Download an image from the internet and display it

=head1 DESCRIPTION

This sample scripts shows how to use an image from another source than a local
file.

=cut

use strict;
use warnings;

use Glib qw(TRUE FALSE);
use Clutter qw(-gtk-init);
use Gtk2 qw(-init);
use Champlain;
use LWP::UserAgent;

exit main();

sub main {
	
	my $window = Gtk2::Window->new();
	$window->set_border_width(10);
	$window->set_title("Champlain - Demo");
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
	my $layer = create_marker_layer($map);
	$map->add_layer($layer);
	
	my $viewport = Gtk2::Viewport->new();
	$viewport->set_shadow_type('etched-in');
	$viewport->add($gtk2_map);

	$vbox->add($viewport);

	$window->add($vbox);
	$window->show_all();
	
	Gtk2->main();
	
	return 0;
}


#
# Adds a marker which has a picture taken from the Internet.
#
sub create_marker_layer {
	my ($map) = @_;
	my $layer = Champlain::Layer->new();

	# Download the image as an actor (Clutter::Texture)
	my $texture = download_texture('http://hexten.net/cpan-faces/potyl.jpg');

	my $marker = Champlain::Marker->new_with_image($texture);
	$marker->set_position(47.130885, -70.764141);
	$layer->add($marker);

	$layer->show();
	return $layer;
}


#
# Download an image from an arbitrary URL and construct a texture
# (Clutter::Texture) with it.
#
sub download_texture {
	my ($url) = @_;

	# Download the image
	my $ua = LWP::UserAgent->new();
	my $response = $ua->get($url);
	if (! $response->is_success) {
		die $response->status_line;
	}

	# Load the image with a Pixbuf Loader
	my $mime = $response->header('content-type');
	my $loader = Gtk2::Gdk::PixbufLoader->new_with_mime_type($mime);
	$loader->write($response->content);
	$loader->close;
	my $pixbuf = $loader->get_pixbuf;

	# Transform the Pixbuf into a Clutter::Texture
	my $actor = Clutter::Texture->new();
	$actor->set_from_rgb_data(
		$pixbuf->get_pixels,
		$pixbuf->get_has_alpha,
		$pixbuf->get_width,
		$pixbuf->get_height,
		$pixbuf->get_rowstride,
		($pixbuf->get_has_alpha ? 4 : 3),
		[]
	);

	return $actor;
}

