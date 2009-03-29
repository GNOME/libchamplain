#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 15;

use Champlain ':coords';


exit tests();


sub tests {
	test_get_set();
	test_add_remove();
	return 0;
}


sub test_get_set {
	my $zoom = Champlain::ZoomLevel->new();
	isa_ok($zoom, 'Champlain::ZoomLevel');
	
	is($zoom->get_width, 0, "Initial width is 0");
	$zoom->set_width(10);
	is($zoom->get_width, 10, "set_width()");
	
	is($zoom->get_height, 0, "Initial height is 0");
	$zoom->set_height(10);
	is($zoom->get_height, 10, "set_height()");
	
	is($zoom->get_zoom_level, 0, "Initial zoom-level is 0");
	$zoom->set_zoom_level(10);
	is($zoom->get_zoom_level, 10, "set_zoom_level()");
	
	isa_ok($zoom->get_actor, "Clutter::Actor", "Initial actor is undef");
}


sub test_add_remove {
	my $zoom = Champlain::ZoomLevel->new();
	isa_ok($zoom, 'Champlain::ZoomLevel');
	
	is($zoom->tile_count, 0, "No tiles by default");
	
	
	# Add some tiles
	my @tiles =  map { Champlain::Tile->new() } (1 .. 3);
	foreach my $tile (@tiles) {
		$zoom->add_tile($tile);
	}
	is($zoom->tile_count, scalar(@tiles), "Tiles count works after adding tiles");
	
	
	# Assert that the tiles are the same
	for (my $i = 0; $i < @tiles; ++$i) {
		my $tile = $tiles[$i];
		is($zoom->get_nth_tile($i), $tile, "Tile $i is valid");
	}
	
	
	# Remove a tile
	$zoom->remove_tile(pop @tiles);
	is($zoom->tile_count, scalar(@tiles), "Tiles count works after removing a tile");
}
