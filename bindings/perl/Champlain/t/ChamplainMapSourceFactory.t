#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 22;

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
	my $factory = Champlain::MapSourceFactory->dup_default();
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
}


sub generic_create {
	my ($factory, $id, $name) = @_;
	my $map = $factory->create($id);
	isa_ok($map, 'Champlain::MapSource');
	is($map->get_id, $id);
	is($map->get_name, $name);
}
