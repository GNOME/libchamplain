#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 35;

use Champlain ':maps';

my $OSM_LICENSE = "(CC) BY 2.0 OpenStreetMap contributors";

exit tests();

sub tests {
	test_map_source_names();
	test_map_factory();
	test_map_register();
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


sub test_map_register {
	my $factory = Champlain::MapSourceFactory->get_default();
	isa_ok($factory, 'Champlain::MapSourceFactory');
	
	# Get the maps available
	my @maps = $factory->get_list();
	ok(@maps >= 5, "Maps factory has the default maps");
	
	# Add a new map
	my $description = {
  	id => 'perl',
		name => 'Perl Fake Map',
		license => 'Artistic License',
		license_uri => 'http://dev.perl.org/licenses/artistic.html',
		min_zoom_level => 0,
		max_zoom_level => 11,
		projection => 'mercator',
	};
	
	$factory->register($description, sub {
		return Champlain::NetworkMapSource->new_full(
			$description->{id},
			$description->{name},
			$description->{license},
			$description->{license_uri},
			$description->{min_zoom_level},
			$description->{max_zoom_level},
			256,
			$description->{projection},
			# This is a copy of OAM
			"http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/#Z#/#X#/#Y#.jpg",
		);
	});

	my @new_maps = $factory->get_list();
	ok(@new_maps == @maps + 1, "Maps factory has an extra map");
	
	my $map = $factory->create('perl');
	isa_ok($map, 'Champlain::MapSource');
	is($map->get_id, $description->{id}, "Created map has the right id");
	is($map->get_name, $description->{name}, "Created map has the right name");
	is($map->get_license, $description->{license}, "Created map has the right license");
	is($map->get_license_uri, $description->{license_uri}, "Created map has the right license_uri");
	is($map->get_min_zoom_level, $description->{min_zoom_level}, "Created map has the right min_zoom_level");
	is($map->get_max_zoom_level, $description->{max_zoom_level}, "Created map has the right max_zoom_level");
	is($map->get_projection, $description->{projection}, "Created map has the right projection");
	is($map->get_tile_size, 256, "Created map has the right tile_size");
	is($map->get_tile_uri(1, 2, 3), "http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/3/1/2.jpg", "Created map has the right tile_uri");
}


sub generic_create {
	my ($factory, $id, $name) = @_;
	my $map = $factory->create($id);
	isa_ok($map, 'Champlain::MapSource');
	is($map->get_id, $id);
	is($map->get_name, $name);
}
