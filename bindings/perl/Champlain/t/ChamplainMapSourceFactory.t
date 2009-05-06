#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 31;

use Champlain ':coords';

my $OSM_LICENSE = "(CC) BY 2.0 OpenStreetMap contributors";

exit tests();

sub tests {
	test_map_source_names();
	test_map_factory();
	return 0;
}


sub test_map_source_names {
	# Map identification
	is(Champlain::MapSourceFactory->OSM_MAPNIK, 'osm::mapnik');
	is(Champlain::MapSourceFactory->OSM_OSMARENDER, 'osm::osmarender');
	is(Champlain::MapSourceFactory->OSM_CYCLE_MAP, 'osm::cyclemap');
	is(Champlain::MapSourceFactory->OAM, 'oam');
	is(Champlain::MapSourceFactory->MFF_RELIEF, 'mff::relief');
}



sub test_map_factory {
	my $factory = Champlain::MapSourceFactory->get_default();
	isa_ok($factory, 'Champlain::MapSourceFactory');
	
	generic_create(
		$factory,
		Champlain::MapSourceFactory->OSM_MAPNIK,
		"OpenStreetMap Mapnik"
	);
	generic_create(
		$factory,
		Champlain::MapSourceFactory->OSM_OSMARENDER,
		"OpenStreetMap Osmarender"
	);
	generic_create(
		$factory,
		Champlain::MapSourceFactory->OSM_CYCLE_MAP,
		"OpenStreetMap Cycle Map"
	);
	generic_create(
		$factory,
		Champlain::MapSourceFactory->OAM,
		"OpenAerialMap"
	);
	generic_create(
		$factory,
		Champlain::MapSourceFactory->MFF_RELIEF,
		"Maps for Free Relief"
	);
	
	# Get the maps available
	my @maps = $factory->get_list();
	ok(@maps >= 5, "Maps factory has the default maps");
	
	# Find the OAM map and check that the it's properly described
	my @found = grep { $_->{id} eq Champlain::MapSourceFactory->OAM } @maps;
	is(scalar(@found), 1);
	if (@found) {
		my ($oam_map) = @found;
		isa_ok($oam_map, 'Champlain::MapSourceDesc');
		is($oam_map->{id}, Champlain::MapSourceFactory->OAM);
		is($oam_map->{name}, 'OpenAerialMap');
		is($oam_map->{license}, "(CC) BY 3.0 OpenAerialMap contributor");
		is($oam_map->{license_uri}, 'http://creativecommons.org/licenses/by/3.0/');
		is($oam_map->{min_zoom_level}, 0);
		is($oam_map->{max_zoom_level}, 17);
		is($oam_map->{projection}, 'mercator');
	}
	else {
		fail("Can't test a Champlain::MapSourceDesc without a map description") for 1 .. 8;
	}
}


sub generic_create {
	my ($factory, $id, $name) = @_;
	my $map = $factory->create($id);
	isa_ok($map, 'Champlain::MapSource');
	is($map->get_id, $id);
	is($map->get_name, $name);
}
