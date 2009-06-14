#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 1;

use Champlain;

exit tests();

sub tests {
	test_generic();
	return 0;
}


sub test_generic {
	my $polygon = Champlain::Polygon->new();
	isa_ok($polygon, 'Champlain::Polygon');
}
