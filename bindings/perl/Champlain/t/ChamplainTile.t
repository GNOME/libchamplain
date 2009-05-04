#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 30;

use Champlain ':coords';


exit tests();


sub tests {
	test_new_empty();
	test_new_full();
	return 0;
}


sub test_new_full {
	my $tile = Champlain::Tile->new_full(50, 75, 514, 1);
	isa_ok($tile, 'Champlain::Tile');
	
	is($tile->get_x(), 50, "get_x() full tile");
	is($tile->get_y(), 75, "get_y() full tile");
	is($tile->get_zoom_level(), 1, "get_zoom_level() full tile");
	is($tile->get_size(), 514, "get_size() full tile");
	is($tile->get_state(), 'init', "get_state() full tile");
	is($tile->get_uri(), '', "get_uri() full tile");
	is($tile->get_filename(), '', "get_filename() full tile");
	isa_ok($tile->get_actor(), 'Clutter::Actor', "get_actor() full tile");
	
	test_all_setters($tile);
}


sub test_new_empty {
	my $tile = Champlain::Tile->new();
	isa_ok($tile, 'Champlain::Tile');
	
	is($tile->get_x(), 0, "get_x() default tile");
	is($tile->get_y(), 0, "get_y() default tile");
	is($tile->get_zoom_level(), 0, "get_zoom_level() default tile");
	is($tile->get_size(), 0, "get_size() default tile");
	is($tile->get_state(), 'init', "get_state() default tile");
	is($tile->get_uri(), '', "get_uri() default tile");
	is($tile->get_filename(), '', "get_filename() default tile");
	isa_ok($tile->get_actor(), 'Clutter::Actor', "get_actor() default tile");
	
	test_all_setters($tile);
}


sub test_all_setters {
	my $tile = Champlain::Tile->new();
	
	$tile->set_x(100);
	is($tile->get_x(), 100, "set_x()");
	
	$tile->set_y(250);
	is($tile->get_y(), 250, "set_y()");
	
	$tile->set_zoom_level(2);
	is($tile->get_zoom_level(), 2, "set_zoom_level()");
	
	$tile->set_size(128);
	is($tile->get_size(), 128, "set_size()");
	
	$tile->set_state('done');
	is($tile->get_state(), 'done', "set_state()");
	
	$tile->set_uri('http://localhost/tile/2/100-250.png');
	is($tile->get_uri(), 'http://localhost/tile/2/100-250.png', "set_uri()");
}
