#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 5;

use Champlain;

exit tests();

sub tests {
	test_empty();
	test_markers();
	return 0;
}


sub test_empty {
	my $layer = Champlain::SelectionLayer->new();
	isa_ok($layer, 'Champlain::Layer');
	
	is($layer->get_selected, undef, "No selection on an empty layer");
	
	my @markers;
	@markers = $layer->get_selected_markers;
	is_deeply(\@markers, [], "No selected markers on an empty layer");
	
	my $count = $layer->count_selected_markers;
	is($count, 0, "Empty marker count on a empty layer");

	my $marker = Champlain::BaseMarker->new();
	
	ok(!$layer->marker_is_selected($marker), "Marker is unselected on an empty layer");
	
	# Can't be tested but at least they are invoked
	$layer->select($marker);
	$layer->unselect($marker);
#	$layer->select_all();
	$layer->unselect_all();

	return 0;
}


sub test_markers {
	my $layer = Champlain::SelectionLayer->new();
	isa_ok($layer, 'Champlain::Layer');
	
	is($layer->get_selected, undef, "No selection on an empty layer");
	
	my @markers;
	@markers = $layer->get_selected_markers;
	is_deeply(\@markers, [], "No selected markers on an empty layer");
	
	my $count = $layer->count_selected_markers;
	is($count, 0, "Empty marker count on a empty layer");

	my $marker = Champlain::BaseMarker->new();
	
	ok(!$layer->marker_is_selected($marker), "Marker is unselected on an empty layer");
	
	# Can't be tested but at least they are invoked
	$layer->select($marker);
	$layer->unselect($marker);
#	$layer->select_all();
	$layer->unselect_all();

	return 0;
}
