#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 110;

use Champlain ':coords';

my $OSM_LICENSE = "(CC) BY 2.0 OpenStreetMap contributors";

exit tests();

sub tests {
	test_osm_mapnik();
	test_osm_cyclemap();
	test_osm_osmarender();
	test_oam();
	test_mff();
	return 0;
}


# OpenStreetMap Mapnik
sub test_osm_mapnik {
	my $label = "OpenStreetMap";
	my $map = Champlain::MapSource->new_osm_mapnik();
	isa_ok($map, 'Champlain::MapSource');
	
	# Map identification
	is($map->get_name, 'OpenStreetMap Mapnik', "$label name");
	is($map->get_min_zoom_level, 0, "$label min zoom");
	is($map->get_max_zoom_level, 18, "$label max zoom");
	is($map->get_tile_size, 256, "$label tile size");
	is($map->get_license, $OSM_LICENSE, "$label license");
	
	# Generic map operations
	generic_map_operations($label, $map);
}


# OpenStreetMap Cycle Map
sub test_osm_cyclemap {
	my $label = "OpenStreetMap (cyclemap)";
	my $map = Champlain::MapSource->new_osm_cyclemap();
	isa_ok($map, 'Champlain::MapSource');
	
	# Map identification
	is($map->get_name, 'OpenStreetMap Cycle Map', "$label name");
	is($map->get_min_zoom_level, 0, "$label min zoom");
	is($map->get_max_zoom_level, 18, "$label max zoom");
	is($map->get_tile_size, 256, "$label tile size");
	is($map->get_license, $OSM_LICENSE, "$label license");
	
	# Generic map operations
	generic_map_operations($label, $map);
}


# OpenStreetMap Osmarender
sub test_osm_osmarender {
	my $label = "OpenStreetMap (osmarender)";
	my $map = Champlain::MapSource->new_osm_osmarender();
	isa_ok($map, 'Champlain::MapSource');
	
	# Map identification
	is($map->get_name, 'OpenStreetMap Osmarender', "$label name");
	is($map->get_min_zoom_level, 0, "$label min zoom");
	is($map->get_max_zoom_level, 18, "$label max zoom");
	is($map->get_tile_size, 256, "$label tile size");
	is($map->get_license, $OSM_LICENSE, "$label license");
	
	# Generic map operations
	generic_map_operations($label, $map);
}

# OpenArialMap
sub test_oam {
	my $label = "OpenArialMap";
	my $map = Champlain::MapSource->new_oam();
	isa_ok($map, 'Champlain::MapSource');
	
	# Map identification
	is($map->get_name, 'OpenArialMap', "$label name");
	is($map->get_min_zoom_level, 0, "$label min zoom");
	is($map->get_max_zoom_level, 17, "$label max zoom");
	is($map->get_tile_size, 256, "$label tile size");
	is($map->get_license, "(CC) BY 3.0 OpenArialMap contributors", "$label license");
	
	# Generic map operations
	generic_map_operations($label, $map);
}


# Maps for Free
sub test_mff {
	my $label = "Maps for Free";
	my $map = Champlain::MapSource->new_mff_relief();
	isa_ok($map, 'Champlain::MapSource');
	
	# Map identification
	is($map->get_name, 'MapsForFree Relief', "$label name");
	is($map->get_min_zoom_level, 0, "$label min zoom");
	is($map->get_max_zoom_level, 11, "$label max zoom");
	is($map->get_tile_size, 256, "$label tile size");
	is(
		$map->get_license,
		"Map data available under GNU Free Documentation license, Version 1.2 or later",
		"$label license"
	);
	
	# Generic map operations
	generic_map_operations($label, $map);
}



# Genereic checks that should work on all map sources
sub generic_map_operations {
	my ($label, $map) = @_;
	
	# Rename of the map
	$map->set_name("No name");
	is($map->get_name, "No name", "Rename the map");
	
	# Relicense the map
	$map->set_license("Free for all!");
	is($map->get_license, "Free for all!", "Relicense the map");
	
	

	# Ask for the X position of the middle point
	is(
		$map->get_x($map->get_min_zoom_level, 0.0),
		$map->get_tile_size/2,
		"$label middle map x"
	);
	is(
		$map->get_longitude($map->get_min_zoom_level, $map->get_tile_size/2),
		0.0,
		"$label middle map longitude"
	);
	
	# Ask for the X position of the point the most to the right
	is(
		$map->get_x($map->get_min_zoom_level, MAX_LONG),
		$map->get_tile_size,
		"$label max map x"
	);
	is(
		$map->get_longitude($map->get_min_zoom_level, $map->get_tile_size),
		MAX_LONG,
		"$label max map longitude"
	);
	
	# Ask for the X position of the point the most to the left
	is(
		$map->get_x($map->get_min_zoom_level, MIN_LONG),
		0,
		"$label min map x"
	);
	is(
		$map->get_longitude($map->get_min_zoom_level, 0),
		MIN_LONG,
		"$label min map longitude"
	);
	
	
	# Ask for the Y position of the point in the middle
	is(
		$map->get_y($map->get_min_zoom_level, 0.0),
		$map->get_tile_size/2,
		"$label middle map y"
	);
	is(
		$map->get_latitude($map->get_min_zoom_level, $map->get_tile_size/2),
		0.0,
		"$label middle map latitude"
	);
	
	# Ask for the Y position of the point the most to the right.
	# Libchamplain is using a "Mercator projection" which has troubles with high
	# latidudes. Values above 85 are not handled properly.
	my $mercator_limit = 85;
	ok(
		$map->get_y($map->get_min_zoom_level, MAX_LAT) > 0,
		"$label max map y"
	);
	ok(
		$map->get_latitude($map->get_min_zoom_level, $map->get_tile_size) < -$mercator_limit,
		"$label max map latitude"
	);
	
	# Ask for the Y position of the point the most to the left
	is(
		$map->get_y($map->get_min_zoom_level, MIN_LAT),
		0,
		"$label min map y"
	);
	ok(
		$map->get_latitude($map->get_min_zoom_level, 0) > $mercator_limit,
		"$label min map latitude"
	);
	
	
	# The map at the first level should have a single tile (1x1)
	is(
		$map->get_row_count($map->get_min_zoom_level),
		1,
		"$label row count at min zoom"
	);
	is(
		$map->get_column_count($map->get_min_zoom_level),
		1,
		"$label column count at min zoom"
	);
}
