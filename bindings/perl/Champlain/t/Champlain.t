#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 5;

use Champlain ':coords';

exit tests();

sub tests {
	test_version();
	test_constants();
	return 0;
}


sub test_version {
	ok($Champlain::VERSION, "Library loaded");
}


sub test_constants {
	is(MIN_LAT,   -90, "MIN_LAT");
	is(MAX_LAT,    90, "MAX_LAT");
	is(MIN_LONG, -180, "MIN_LONG");
	is(MAX_LONG,  180, "MAX_LONG");
}
