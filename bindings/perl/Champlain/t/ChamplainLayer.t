#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 1;

use Champlain;

exit tests();

sub tests {
	my $layer = Champlain::Layer->new();
	isa_ok($layer, 'Champlain::Layer');
	return 0;
}
