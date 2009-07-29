#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 6;

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

	ok(!$layer->get('visible'), "Layer is not visible at start");
	$layer->show();
	ok($layer->get('visible'), "show()");
	$layer->hide();
	ok(!$layer->get('visible'), "hide()");

	return 0;
}
