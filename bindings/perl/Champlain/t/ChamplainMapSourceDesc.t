#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 18;

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
	my @found = grep { $_->get_id eq Champlain::MapSourceFactory->OAM } @maps;
	is(scalar(@found), 1, "Found a single map matching OAM");
	if (! @found) {
		fail("Can't test a Champlain::MapSourceDesc without a map description") for 1 .. 22;
		return;
	}

	# Getters
	my ($oam) = @found;
	isa_ok($oam, 'Champlain::MapSourceDesc');
	is($oam->get_id, Champlain::MapSourceFactory->OAM, "get_id()");
	is($oam->get_name, 'OpenAerialMap', "get_name()");
	is($oam->get_license, "(CC) BY 3.0 OpenAerialMap contributors", "get_license()");
	is($oam->get_license_uri, 'http://creativecommons.org/licenses/by/3.0/', "get_license_uri()");
	is($oam->get_min_zoom_level, 0, "get_min_zoom_level()");
	is($oam->get_max_zoom_level, 17, "get_max_zoom_level()");
	is($oam->get_projection, 'mercator', "get_projection()");
	
	
	# Setters
	$oam->set_id('test');
	is($oam->get_id, 'test', "set_id()");
	
	$oam->set_name("new name");
	is($oam->get_name, "new name", "set_name()");
	
	$oam->set_license("free for all");
	is($oam->get_license, "free for all", "set_license()");
	
	$oam->set_license_uri('file:///tmp/free.txt');
	is($oam->get_license_uri, 'file:///tmp/free.txt', "set_license_uri()");
	
	$oam->set_min_zoom_level(2);
	is($oam->get_min_zoom_level, 2, "set_min_zoom_level()");
	
	$oam->set_max_zoom_level(4);
	is($oam->get_max_zoom_level, 4, "set_max_zoom_level()");
	
	# There are no other projections now
	$oam->set_projection('mercator');
	is($oam->get_projection, 'mercator', "set_projection()");
}
