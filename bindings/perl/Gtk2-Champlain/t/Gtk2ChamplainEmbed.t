#!/usr/bin/perl

use strict;
use warnings;

use Test::More tests => 2;
use Gtk2::Clutter '-init';

use Gtk2::Champlain;


exit tests();


sub tests {
	test_generic();
	return 0;
}


#
# Test some default functionality.
#
sub test_generic {
	my $embed = Gtk2::Champlain::Embed->new();
	isa_ok($embed, 'Gtk2::Champlain::Embed');
	
	my $view = $embed->get_view();
	isa_ok($view, 'Champlain::View');
}
