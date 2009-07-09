#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 25;

use Champlain;
use Data::Dumper;

exit tests();

sub tests {
	test_empty();
#	test_markers_single();

	test_markers_multiple();
	return 0;
}


sub test_empty {
	my $layer = Champlain::SelectionLayer->new();
	isa_ok($layer, 'Champlain::Layer');

	my $count;
	my @markers;

	is($layer->get_selected, undef, "[empty] get_selected()");

	# In single mode get_selected_markers doesn't work
	@markers = $layer->get_selected_markers;
	is_deeply(\@markers, [], "[empty] get_selected_markers() list context");
	$count = $layer->get_selected_markers;
	is($count, 0, "[empty] get_selected_markers() scalar context");

	$count = $layer->count_selected_markers;
	is($count, 0, "[empty] count_selected_markers()");

	my $marker = Champlain::BaseMarker->new();
	ok(!$layer->marker_is_selected($marker), "[empty] marker_is_selected()");

	# Can't be tested but at least they are invoked
	$layer->select($marker);
	$layer->unselect($marker);
#	$layer->select_all();
	$layer->unselect_all();

	return 0;
}


sub test_markers_multiple {
	my $layer = Champlain::SelectionLayer->new();
	isa_ok($layer, 'Champlain::Layer');


	my @layer_markers = (
		Champlain::BaseMarker->new(),
		Champlain::BaseMarker->new(),
		Champlain::BaseMarker->new(),
		Champlain::BaseMarker->new(),
	);

	# Add the markers and select a few markers
	foreach my $marker (@layer_markers) {
		$layer->add($marker);
	}
	$layer->select($layer_markers[1]);
	$layer->select($layer_markers[3]);


	# This doesn't work in multiple mode
	is($layer->get_selected, undef, "[multiple] get_selected()");


	my @markers;
	@markers = $layer->get_selected_markers;
	is_deeply(\@markers, [$layer_markers[1], $layer_markers[3]], "[multiple] get_selected_markers() list context");

	my $count = $layer->count_selected_markers;
	is($count, 2, "[multiple] count_selected_markers()");

	my $marker = Champlain::BaseMarker->new();

	# Check wich markers are selected
	ok(!$layer->marker_is_selected($marker), "[multiple] marker_is_selected() maker not in set");
	ok(!$layer->marker_is_selected($layer_markers[0]), "[multiple] marker_is_selected() maker not selected");
	ok(!$layer->marker_is_selected($layer_markers[2]), "[multiple] marker_is_selected() maker not selected");
	ok($layer->marker_is_selected($layer_markers[1]), "[multiple] marker_is_selected() selected");
	ok($layer->marker_is_selected($layer_markers[3]), "[multiple] marker_is_selected() selected");


	# Select a new marker
	$layer->select($marker);
	ok($layer->marker_is_selected($marker), "[multiple] select() maker not in set");
	$count = $layer->count_selected_markers;
	is($count, 3, "[multiple] count_selected_markers() with a new marker");
	is_deeply(
		[ $layer->get_selected_markers ],
		[$layer_markers[1], $layer_markers[3], $marker],
		"[multiple] get_selected_markers() list context"
	);


	# Select again one of the selected markers, this unselects it
	$layer->select($marker);
	ok(!$layer->marker_is_selected($marker), "[multiple] select() deselects an already selected marker");
	$count = $layer->count_selected_markers;
	is($count, 2, "[multiple] count_selected_markers() with a deselected marker");
	is_deeply(
		[ $layer->get_selected_markers ],
		[$layer_markers[1], $layer_markers[3]],
		"[multiple] get_selected_markers() list context"
	);


	# Remove a marker
	$layer->unselect($layer_markers[1]);
	$count = $layer->count_selected_markers;
	is($count, 1, "[multiple] count_selected_markers() after unselect()");
	is_deeply(
		[ $layer->get_selected_markers ],
		[$layer_markers[3]],
		"[multiple] get_selected_markers() list context"
	);

	# Remove all markers
	$layer->unselect_all();
	$count = $layer->count_selected_markers;
	is($count, 0, "[multiple] count_selected_markers() after count_selected_markers()");
	is_deeply(
		[ $layer->get_selected_markers ],
		[],
		"[multiple] get_selected_markers() list context"
	);

	return 0;
}
