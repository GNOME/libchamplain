#!/usr/bin/perl

use strict;
use warnings;

use Glib qw(TRUE FALSE);
use Clutter qw(-threads-init -init);
use Champlain;
use Data::Dumper;
use FindBin;
use File::Spec;

exit main();


sub main {
	
	my $stage = Clutter::Stage->get_default();
	$stage->set_size(800, 600);
	
	# Create the map view
	my $map = Champlain::View->new();
	$map->set('scroll-mode', 'kinetic');
#	$map->set('map-source', 'mapsforfree-relief');
	$map->set_size(800, 600);
	
	# Create the markers and marker layer
	my $layer = create_marker_layer($map);
	$map->add_layer($layer);
	
	# Finish initializing the map view
	$map->set_property("zoom-level", 7);
	$map->center_on(45.466, -73.75);
	
	# Middle click to get the location in the map
	$map->set_reactive(TRUE);
	$map->signal_connect_after("button-release-event", \&map_view_button_release_cb, $map);

	$stage->add($map);
	$stage->show_all();
	
	Clutter->main();
	
	return 0;
}


sub create_marker_layer {
	my ($map) = @_;
	my $layer = Champlain::Layer->new();

	my $orange = Clutter::Color->new(0xf3, 0x94, 0x07, 0xbb);
	my $white = Clutter::Color->new(0xff, 0xff, 0xff, 0xff);
	
	my $marker;
	
	$marker = Champlain::Marker->new_with_label("Montr\x{e9}al", "Airmole 14", undef, undef);
	$marker->set_position(45.528178, -73.563788);
	$marker->set_reactive(TRUE);
	$marker->signal_connect_after("button-release-event", \&marker_button_release_cb, $map);
	$layer->add($marker);

	$marker = Champlain::Marker->new_with_label("New York", "Sans 15", $white, undef);
	$marker->set_position(40.77, -73.98);
	$layer->add($marker);

	my $file = File::Spec->catfile($FindBin::Bin, 'who.png');
	eval {
		$marker = Champlain::Marker->new_with_image_full($file, 40, 40, 20, 20);
		$marker->set_position(47.130885, -70.764141);
		$layer->add($marker);
	};
	if (my $error = $@) {
		warn "Failed to load image $file because $error";
	}

	$layer->show();
	return $layer;
}


sub marker_button_release_cb {
	my ($marker, $event, $map) = @_;

	return FALSE if $event->button != 1 || $event->click_count > 1;
	
	print "Montreal was clicked\n";

	return TRUE;
}


sub map_view_button_release_cb {
	my ($actor, $event, $map) = @_;
	return FALSE if $event->button != 2 || $event->click_count > 1;

	my ($lat, $lon) = $map->get_coords_from_event($event);
	printf "Map was clicked at %f, %f\n", $lat, $lon;
	return TRUE;
}

