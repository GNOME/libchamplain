#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 34;

use Champlain;

my $DEFAULT_FILL_COLOR = Champlain::Polygon->new()->get_fill_color;
my $DEFAULT_STROKE_NAME = Champlain::Polygon->new()->get_stroke_color;

exit tests();

sub tests {
	test_empty();
	test_setters();
#	test_points();
	return 0;
}


sub test_empty {
	my $polygon = Champlain::Polygon->new();
	isa_ok($polygon, 'Champlain::Polygon');
	
	my @points = $polygon->get_points();
	is_deeply(\@points, [], "No points on a new polygon");
	
	$polygon->clear_points();
	is_deeply(\@points, [], "No points on a cleared polygon");
	
	is_color($polygon->get_fill_color, $DEFAULT_FILL_COLOR, "fill_color is set on a new polygon");
	is_color($polygon->get_stroke_color, $DEFAULT_STROKE_NAME, "stroke_color is set on a new polygon");
	
	ok(!$polygon->get_fill, "fill is set on a new polygon");
	ok($polygon->get_stroke, "stroke is set on a new polygon");
	is($polygon->get_stroke_width, 2, "stroke_width is set on a new polygon");
	
	# Call these methods hopping that they won't crash.
	$polygon->show();
	$polygon->hide();
}


sub test_setters {
	my $polygon = Champlain::Polygon->new();
	isa_ok($polygon, 'Champlain::Polygon');


	my $color;
	
	$color = Clutter::Color->new(0xaa, 0xdd, 0x37, 0xbb);
	$polygon->set_fill_color($color);
	is_color($polygon->get_fill_color, $color, "set_fill_color");
	
	$color = Clutter::Color->new(0x44, 0x33, 0x23, 0x2b);
	$polygon->set_stroke_color($color);
	is_color($polygon->get_stroke_color, $color, "set_stroke_color");
	
	{
		my $old_fill = $polygon->get_fill;
		$polygon->set_fill(!$old_fill);
		is($polygon->get_fill, !$old_fill, "set_fill()");
	}
	
	{
		my $old_stroke = $polygon->get_stroke;
		$polygon->set_stroke(!$old_stroke);
		is($polygon->get_stroke, !$old_stroke, "set_stroke()");
	}

	{	
		my $old_stroke_width = $polygon->get_stroke_width;
		$polygon->set_stroke_width($old_stroke_width + 2);
		is($polygon->get_stroke_width, $old_stroke_width + 2, "set_stroke_width()");
	}
}


#
# Assert that two colors are identical.
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
