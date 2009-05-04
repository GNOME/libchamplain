#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 110;

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
	is(Champlain::MapSourceFactory->OSM_MAPNIK, 'OpenStreetMap Mapnik');
	is(Champlain::MapSourceFactory->OSM_OSMARENDER, 'OpenStreetMap Osmarender');
	is(Champlain::MapSourceFactory->OSM_CYCLEMAP, 'OpenStreetMap CycleMap');
	is(Champlain::MapSourceFactory->OAM, 'OpenAerialMap');
	is(Champlain::MapSourceFactory->MFF_RELIEF, 'MapsForFree Relief');
}



sub test_map_factory {
	my $factory = Champlain::MapSourceFactory->get_default();
	isa_ok($factory, 'Champlain::MapSourceFactory');
	
	generic_create($factory, Champlain::MapSourceFactory->OSM_MAPNIK);
	generic_create($factory, Champlain::MapSourceFactory->OSM_OSMARENDER);
	generic_create($factory, Champlain::MapSourceFactory->OSM_CYCLEMAP);
	generic_create($factory, Champlain::MapSourceFactory->OAM);
	generic_create($factory, Champlain::MapSourceFactory->MFF_RELIEF);
}


sub generic_create {
	my ($factory, $id) = @_;
	my $map = $factory->create($id);
	isa_ok($map, 'Champlain::MapSource');
	is($map->get_name, $id);
}
