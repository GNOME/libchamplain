#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 44;

use Champlain ':coords';


exit tests();


sub tests {
	test_generic_in_stage();
	test_zoom();
	test_zoom_in_stage();
	test_event();
	return 0;
}


#
# Test some default functionality when the view is in a stage
#
sub test_generic_in_stage {
	my $view = get_view(TRUE);
	
	# center_on() can be tested by checking the properties latitude and longitude.
	# And even then, the values set are not the ones returned
	my $latitude = $view->get('latitude');
	my $longitude = $view->get('longitude');
	$view->center_on(48.144722, 17.112778);
	ok($view->get('latitude') != $latitude, "center_on() changed latitude");
	ok($view->get('longitude') != $longitude, "center_on() changed longitude");
	
	# set_size() can be tested by checking the properties width and height
	is($view->get('width'), 800, "original width");
	is($view->get('height'), 600, "original height");
	$view->set_size(600, 400);
	is($view->get('width'), 600, "set_size() changed width");
	is($view->get('height'), 400, "set_size() changed height");
	
	
	# Can't be tested but at least we check that it doesn't crash when invoked
	my $layer = Champlain::Layer->new();
	$view->add_layer($layer);
	
	
	# Change the map source (get a different map source)
	my $source_original = $view->get('map-source');
	my $source_new = Champlain::MapSource->new_osm_mapnik();
	if ($source_original->get_name eq $source_new->get_name) {
		# Same kind of map source, take another one
		$source_new = Champlain::MapSource->new_oam();
	}
	$view->set_map_source($source_new);
	is($view->get('map-source'), $source_new, "Change map source");
}


#
# Test the zoom functionality with a view that's in a stage.
#
sub test_zoom_in_stage {
	my $view = get_view(TRUE);
	
	
	# Zoom in
	is($view->get('zoom-level'), 0, "original zoom-level");
	$view->zoom_in();
	is($view->get('zoom-level'), 1, "zoom-in once");
	$view->zoom_in();
	is($view->get('zoom-level'), 2, "zoom-in twice");
	
	# Zoom out
	$view->zoom_out();
	is($view->get('zoom-level'), 1, "zoom-out once");
	$view->zoom_out();
	is($view->get('zoom-level'), 0, "zoom-out twice");
	
	my $map_source = $view->get('map-source');
	
	# Zoom out past the min zoom level
	my $min = $map_source->get_min_zoom_level;
	$view->set_zoom_level($min);
	is($view->get('zoom-level'), $min, "zoom-out to the minimal level");

	$view->set("zoom-level", $min);
	is($view->get('zoom-level'), $min, "set('zoom-level') to the minimal level");

	$view->zoom_out();
	is($view->get('zoom-level'), $min, "zoom-out past minimal level has no effect");
	
	
	# Zoom in after the max zoom level
	my $max = $map_source->get_max_zoom_level;
	$view->set_zoom_level($max);
	is($view->get('zoom-level'), $max, "zoom-in to the maximal level");

	$view->set("zoom-level", $max);
	is($view->get('zoom-level'), $max, "set('zoom-level') to the maximal level");

	$view->zoom_in();
	is($view->get('zoom-level'), $max, "zoom-in past maximal level has no effect");
	
	# Go to the middle zoom level
	my $middle = int( ($max - $min) / 2 );
	$view->set_zoom_level($middle);
	is($view->get('zoom-level'), $middle, "set zoom to the middle level");

	$view->set("zoom-level", $middle);
	is($view->get('zoom-level'), $middle, "set('zoom-level', (max-min)/2) to the middle level");
	
	
	# Try to set directly the zoom level to a value inferior to min level
	$view->set_zoom_level($min - 1);
	is($view->get('zoom-level'), $middle, "set zoom (min - 1) has no effect");

	# NOTE: This gives a warning because -1 out of range for property `zoom-level'
	#$view->set("zoom-level", $min - 1);
	#is($view->get('zoom-level'), $middle, "set('zoom-level', min - 1) has no effect");
	
	# Try to set directly the zoom level to a valu superior to max level
	$view->set_zoom_level($max + 1);
	is($view->get('zoom-level'), $middle, "set zoom (max + 1) has no effect");

	$view->set("zoom-level", $max + 1);
	is($view->get('zoom-level'), $middle, "set('zoom-level', max + 1) has no effect");
}


#
# Test the zoom functionality with a view that's not in a stage. If the view is
# not connected to a stage most operations will not work.
#
sub test_zoom {
	my $view = get_view();
	
	
	# Zoom in
	is($view->get('zoom-level'), 0, "original zoom-level");
	$view->zoom_in();
	is($view->get('zoom-level'), 0, "zoom-in has no effect");
	
	# Zoom out
	$view->zoom_out();
	is($view->get('zoom-level'), 0, "zoom-out has no effect");

	
	# Zoom to a random place
	$view->set_zoom_level(1);
	is($view->get('zoom-level'), 0, "set_zoom_level has no effect");
	$view->set("zoom-level", 1);
	is($view->get('zoom-level'), 0, "set('zoom-level') has no effect");

	
	my $map_source = $view->get('map-source');
	
	# Zoom out past the min zoom level
	my $min = $map_source->get_min_zoom_level;
	$view->set_zoom_level($min);
	is($view->get('zoom-level'), 0, "zoom-out to the minimal level has no effect");
	
	$view->set("zoom-level", $min);
	is($view->get('zoom-level'), 0, "set('zoom-level') to the minimal level has no effect");
	
	$view->zoom_out();
	is($view->get('zoom-level'), 0, "zoom-out past minimal level has no effect");
	
	
	# Zoom in after the max zoom level
	my $max = $map_source->get_max_zoom_level;
	$view->set_zoom_level($max);
	is($view->get('zoom-level'), 0, "zoom-in to the maximal level has no effect");
	
	$view->set("zoom-level", $max);
	is($view->get('zoom-level'), 0, "set('zoom-level') to the maximal level has no effect");

	$view->zoom_in();
	is($view->get('zoom-level'), 0, "zoom-in past maximal level has no effect");
	
	
	# Try to set directly the zoom level to a value inferior to min level
	$view->set_zoom_level($min - 1);
	is($view->get('zoom-level'), 0, "set zoom (min - 1) has no effect");
	
	# NOTE: This gives a warning because -1 out of range for property `zoom-level'
	#$view->set("zoom-level", $min - 1);
	#is($view->get('zoom-level'), 0, "set('zoom-level', min -1) has no effect");
	
	# Try to set directly the zoom level to a valu superior to max level
	$view->set_zoom_level($max + 1);
	is($view->get('zoom-level'), 0, "set zoom (max + 1) has no effect");
}


#
# Test getting the coordinates from an event.
#
# This tests simulates that the user clicked at the coordinate (0, 0) (where
# Greenwich meets the Equator). In order to simulate this, the test sets the
# view to be as big as the first tile and will simulate a click in the middle of
# the tile. Because the computations are made with a projection a slight error
# threshold will be accepted.
#
sub test_event {
	my $view = get_view();
	
	my $map_source = $view->get('map-source');
	my $size = $map_source->get_tile_size;
	
	# NOTE: At the moment this works only if the view is in a stage and if
	# show_all() was called
	my $stage = Clutter::Stage->get_default();
	$stage->set_size($size, $size);
	$view->set_size($size, $size);
	$stage->add($view);
	$stage->show_all();
	
	# Create a fake event in the middle of the tile
	my $event = Clutter::Event->new('button_press');
	$event->x($size/2);
	$event->y($size/2);
	is($event->x, $size/2);
	is($event->y, $size/2);
	
	my ($latitude, $longitude) = $view->get_coords_from_event($event);
	ok($latitude >= -2.0 && $latitude <= 2.0, "get_coords_from_event() latitude");
	ok($longitude >= -2.0 && $longitude <= 2.0, "get_coords_from_event() longitude");
}


sub get_view {
	my ($in_stage) = @_;

	my $view = Champlain::View->new();
	isa_ok($view, 'Champlain::View');
	
	return $view unless $in_stage;
	
	my $stage = Clutter::Stage->get_default();
	$stage->set_size(800, 600);
	$view->set_size($stage->get_size);
	$stage->add($view);
	
	return $view;
}
