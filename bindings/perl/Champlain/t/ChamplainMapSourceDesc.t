#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 22;
use Test::Exception;

use Champlain ':maps';

exit tests();

sub tests {
	test_get_set();
	return 0;
}


sub test_get_set {
	my $factory = Champlain::MapSourceFactory->dup_default();
	isa_ok($factory, 'Champlain::MapSourceFactory');
	
	# Get the maps available
	my @maps = $factory->dup_list();
	ok(@maps >= 5, "Maps factory has the default maps");
	
	# Find the OAM map and check that the it's properly described
	my @found = grep { $_->id eq Champlain::MapSourceFactory->OAM } @maps;
	is(scalar(@found), 1, "Found a single map matching OAM");
	if (! @found) {
		fail("Can't test a Champlain::MapSourceDesc without a map description") for 1 .. 22;
		return;
	}

	# Getters
	my ($oam) = @found;
	isa_ok($oam, 'Champlain::MapSourceDesc');
	is($oam->id, Champlain::MapSourceFactory->OAM, "get id()");
	is($oam->name, 'OpenAerialMap', "get name()");
	is($oam->license, "(CC) BY 3.0 OpenAerialMap contributors", "get license()");
	is($oam->license_uri, 'http://creativecommons.org/licenses/by/3.0/', "get license_uri()");
	is($oam->min_zoom_level, 0, "get min_zoom_level()");
	is($oam->max_zoom_level, 17, "get max_zoom_level()");
	is($oam->projection, 'mercator', "get projection()");
	is($oam->uri_format, 'http://tile.openaerialmap.org/tiles/1.0.0/openaerialmap-900913/#Z#/#X#/#Y#.jpg', "get uri_format()");
	
	# Setters
	$oam->id('test');
	is($oam->id, 'test', "set id()");
	
	$oam->name("new name");
	is($oam->name, "new name", "set name()");
	
	$oam->license("free for all");
	is($oam->license, "free for all", "set license()");
	
	$oam->license_uri('file:///tmp/free.txt');
	is($oam->license_uri, 'file:///tmp/free.txt', "set license_uri()");
	
	$oam->min_zoom_level(2);
	is($oam->min_zoom_level, 2, "set min_zoom_level()");
	
	$oam->max_zoom_level(4);
	is($oam->max_zoom_level, 4, "set max_zoom_level()");
	
	# There are no other projections now, we have to trust that the setter works
	$oam->projection('mercator');
	is($oam->projection, 'mercator', "set projection()");

	$oam->uri_format('http://tile.oam.org/tiles/#Z#/#X#/#Y#.jpg');
	is($oam->uri_format, 'http://tile.oam.org/tiles/#Z#/#X#/#Y#.jpg', "set uri_format()");
	
	
	# The constructor is not yet available in the perl bindings
	throws_ok { $oam->constructor } qr/\Qdesc->constructor() isn't implemented yet/, "get constructor() isn't implemented";
	throws_ok { $oam->constructor(sub{}) } qr/\Qdesc->constructor(\&code_ref)/, "set constructor() isn't implemented";
}
