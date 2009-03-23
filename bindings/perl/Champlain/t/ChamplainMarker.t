#!/usr/bin/perl

use strict;
use warnings;

use Clutter::TestHelper tests => 7;

use Champlain ':coords';
use File::Spec;

# Path to a valid image
my $FILENAME = File::Spec->catfile('examples', 'images', 'who.png');


exit tests();


sub tests {
	test_new();
	test_new_with_label();
	test_new_with_image();
	test_new_with_image_full();
	return 0;
}


sub test_new {
	my $marker = Champlain::Marker->new();
	isa_ok($marker, 'Champlain::Marker');
}


sub test_new_with_label {
	# Create a label without specifying the colors
	my $marker_label = Champlain::Marker->new_with_label(
		"Home", "Airmole 14", undef, undef
	);
	isa_ok($marker_label, 'Champlain::Marker');
	
	# Create a label without specifying the colors
	my $marker_label_color = Champlain::Marker->new_with_label(
		"Home", "Airmole 14",
		Clutter::Color->new(0xf3, 0x94, 0x07, 0xbb), # orange
		Clutter::Color->new(0xff, 0xff, 0xff, 0xff),
	);
	isa_ok($marker_label_color, 'Champlain::Marker');
}


sub test_new_with_image {
	my $marker = Champlain::Marker->new_with_image($FILENAME);
	isa_ok($marker, 'Champlain::Marker');
	
	# Assert that using a file that doesn't exist throws an exception
	eval {
		$marker = Champlain::Marker->new_with_image("does-not-exist.gif");
	};
	isa_ok($@, "Glib::File::Error");
}


sub test_new_with_image_full {
	my $marker = Champlain::Marker->new_with_image_full(
		$FILENAME,
		64, 64, # width, height
		10, 10 # x, y
	);
	isa_ok($marker, 'Champlain::Marker');
	
	# Assert that using a file that doesn't exist throws an exception
	eval {
		$marker = Champlain::Marker->new_with_image_full(
			"does-not-exist.gif", 10, 10, 1, 1
		);
	};
	isa_ok($@, "Glib::File::Error");
}
