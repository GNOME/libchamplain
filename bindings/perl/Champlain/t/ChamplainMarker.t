#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 28;
use Test::Builder;

use Champlain ':coords';
use File::Spec;

# Path to a valid image
my $FILENAME = File::Spec->catfile('examples', 'images', 'who.png');


exit tests();


sub tests {
	test_new();
	test_new_with_text();
	test_new_with_image();
	test_new_full();
	return 0;
}


sub test_new {
	my $marker = Champlain::Marker->new();
	isa_ok($marker, 'Champlain::Marker');
}


sub test_new_with_text {

	my $marker;

	# Create a label without mandatory arguments
	$marker = Champlain::Marker->new_with_text(
		"Home", undef, undef, undef
	);
	isa_ok($marker, 'Champlain::Marker');
	is($marker->get_text, 'Home', "new_with_text() sets 'text'");
	ok($marker->get_font_name, "new_with_text(font = undef) sets 'font_name'");
	isa_ok($marker->get_text_color, 'Clutter::Color', "new_with_text(text_color = undef) sets 'text_color'");
	isa_ok($marker->get_color, 'Clutter::Color', "new_with_text(color = undef) sets 'color'");


	# Create a label by specifying the colors
	$marker = Champlain::Marker->new_with_text(
		"Bratislava",
		"Airmole 14",
		Clutter::Color->new(0xf3, 0x94, 0x07, 0xbb), # orange
		Clutter::Color->new(0xff, 0xff, 0xff, 0xff), # white
	);
	isa_ok($marker, 'Champlain::Marker');
	is($marker->get_text, 'Bratislava', "new_with_text() sets 'text'");
	is($marker->get_font_name, 'Airmole 14', "new_with_text() sets 'font_name'");
	is_color(
		$marker->get_text_color,
		Clutter::Color->new(0xf3, 0x94, 0x07, 0xbb),
		"new_with_text() sets 'text_color'"
	);
	is_color(
		$marker->get_color,
		Clutter::Color->new(0xff, 0xff, 0xff, 0xff),
		"new_with_text() sets 'color'"
	);
}


sub test_new_with_image {
	my $marker = Champlain::Marker->new_with_image($FILENAME);
	isa_ok($marker, 'Champlain::Marker');
	isa_ok($marker->get_image, 'Clutter::Actor');
	
	# Assert that using a file that doesn't exist throws an exception
	eval {
		$marker = Champlain::Marker->new_with_image("does-not-exist.gif");
	};
	isa_ok($@, "Glib::File::Error");
}


sub test_new_full {
	my $marker = Champlain::Marker->new_full(
		"hello",
		$FILENAME,
	);
	isa_ok($marker, 'Champlain::Marker');
	is($marker->get_text, 'hello', "new_full() sets 'text'");
	isa_ok($marker->get_image, 'Clutter::Actor');
	
	# Assert that using a file that doesn't exist throws an exception
	$@ = undef;	
	eval {
		$marker = Champlain::Marker->new_full(
			"test",			
			"does-not-exist.gif",
		);
	};
	isa_ok($@, "Glib::File::Error");
}


#
# Compare colors.
#
sub is_color {
	my ($got, $expected, $message) = @_;
	my $tester = Test::Builder->new();
	my $are_colors = 1;
	$are_colors &= $tester->is_eq(ref($got), 'Clutter::Color', "$message, got is a Clutter::Color");
	$are_colors &= $tester->is_eq(ref($expected), 'Clutter::Color', "$message, expected a Clutter::Color");

	if (! $are_colors) {
		$tester->ok(0, "$message, can't compare color components") for 1 .. 4;
		return;
	}

	$tester->is_num($got->red, $expected->red, "$message, red matches");
	$tester->is_num($got->green, $expected->green, "$message, green matches");
	$tester->is_num($got->blue, $expected->blue, "$message, blue matches");
	$tester->is_num($got->alpha, $expected->alpha, "$message, alpha matches");
}

