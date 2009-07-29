#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 3;

use Champlain;

exit tests();

sub tests {
	my $layer = Champlain::Layer->new();
	isa_ok($layer, 'Champlain::Layer');

	my $marker = Champlain::BaseMarker->new();
	is_deeply(
		[$layer->get_children],
		[],
		"No children at start"
	);
	$layer->add_marker($marker);
	is_deeply(
		[$layer->get_children],
		[$marker],
		"Layer has a marker after add_marker"
	);

	return 0;
}
