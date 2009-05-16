#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 11;

use Champlain ':coords';

exit tests();

sub tests {
	test_version();
	test_constants();
	return 0;
}


sub test_version {
	ok($Champlain::VERSION, "Library loaded");

	ok(defined Champlain::MAJOR_VERSION, "MAJOR_VERSION exists");
	ok(defined Champlain::MINOR_VERSION, "MINOR_VERSION exists");
	ok(defined Champlain::MICRO_VERSION, "MICRO_VERSION exists");

	ok (Champlain->CHECK_VERSION(0,0,0), "CHECK_VERSION pass");
	ok (!Champlain->CHECK_VERSION(50,0,0), "CHECK_VERSION fail");

	my @version = Champlain->GET_VERSION_INFO;
	my @expected = (
		Champlain::MAJOR_VERSION,
		Champlain::MINOR_VERSION,
		Champlain::MICRO_VERSION,
	);
	is_deeply(\@version, \@expected, "GET_VERSION_INFO");
}


sub test_constants {
	is(MIN_LAT,   -90, "MIN_LAT");
	is(MAX_LAT,    90, "MAX_LAT");
	is(MIN_LONG, -180, "MIN_LONG");
	is(MAX_LONG,  180, "MAX_LONG");
}
