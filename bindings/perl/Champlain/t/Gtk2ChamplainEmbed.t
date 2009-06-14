#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 3, sub_module => 'gtk';
use Gtk2 '-init';

use Champlain ':coords';


exit tests();


sub tests {
	test_all();
	return 0;
}

sub test_all {

	my $view = Champlain::View->new();
	isa_ok($view, 'Champlain::View');

	my $embed = Gtk2::Champlain::ViewEmbed->new($view);
	isa_ok($embed, 'Gtk2::Champlain::ViewEmbed');
	
	is($embed->get_view, $view, "get_view()");
}
