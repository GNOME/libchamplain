#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 4;

use Champlain ':coords';


exit tests();


sub tests {
	test_all();
	return 0;
}


sub test_all {
	my $map_source = Champlain::NetworkMapSource->new_full(
		'test::fake-map',
		'Fake map',
		'free',
		'http://www.it-is-free.org/license.txt',
		0,
		10,
		128,
		'mercator',
		'http://www.it-is-free.org/tiles/#Z#/#X#-#Y#.png'
	);
	isa_ok($map_source, 'Champlain::NetworkMapSource');
	
	is($map_source->get_id, 'test::fake-map');
	is($map_source->get_name, 'Fake map');
	
	is(
		$map_source->get_tile_uri(100, 200, 3),
		'http://www.it-is-free.org/tiles/3/100-200.png',
		"get_tile_uri()"
	);
}
