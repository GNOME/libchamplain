#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 9;

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

	# Too hard to test, but at least we call them
	$layer->animate_in_all_markers();
	$layer->animate_out_all_markers();

	# Show/Hide the markers
	ok($marker->get('visible'), "marker is not visible");
	$layer->hide_all_markers();
	ok(!$marker->get('visible'), "hide_all_markers()");


	$layer->show_all_markers();
	ok($marker->get('visible'), "show_all_markers()");
	return 0;
}
