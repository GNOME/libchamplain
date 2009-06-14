#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 2, sub_module => 'gtk';
use Gtk2 '-init';

use Champlain ':coords';


exit tests();


sub tests {
	test_all();
	return 0;
}

sub test_all {

	my $embed = Gtk2::ChamplainEmbed->new();
	isa_ok($embed, 'Gtk2::ChamplainEmbed');
	
	my $view = $embed->get_view;
	isa_ok($view, 'Champlain::View');
}
