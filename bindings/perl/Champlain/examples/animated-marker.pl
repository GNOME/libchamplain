#!/usr/bin/perl

use strict;
use warnings;

use Glib qw(TRUE FALSE);
use Clutter qw(-threads-init -init);
use Champlain;
use Math::Trig ':pi';


my $MARKER_SIZE = 10;


exit main();


sub main {
	
	my $stage = Clutter::Stage->get_default();
	$stage->set_size(800, 600);
	
	# Create the map view
	my $map = Champlain::View->new();
	$map->set_size($stage->get_size);
	$stage->add($map);
	
	
	# Create the marker layer
	my $layer = Champlain::Layer->new();
	$map->add_layer($layer);
	
	# Create the marker
	my $marker = create_marker();
	$layer->add($marker);
	
	
	# Finish initializing the map view
	$map->set_property("zoom-level", 5);
	$map->set_property("scroll-mode", 'kinetic');
	$map->center_on(45.466, -73.75);
	

	$stage->show_all();
	
	Clutter->main();
	
	return 0;
}


#
# The marker is drawn with cairo. It is composed of 1 static filled circle and 1
# stroked circle animated as an echo.
#
sub create_marker {
	my $marker = Champlain::Marker->new();
	
	# Static filled circle
	create_static_circle($marker);
	
	
	# Echo circle
	my $echo = create_echo_circle($marker);
	
	##
	## Animate the echo circle
	##
	my $timeline = Clutter::Timeline->new_for_duration(1000);
	$timeline->set_loop(TRUE);
	my $alpha = Clutter::Alpha->new($timeline, \&Clutter::Alpha::sine_inc);
	
	my $behaviour = Clutter::Behaviour::Scale->new($alpha, 0.5, 0.5, 2.0, 2.0);
	$behaviour->apply($echo);
	
	$behaviour = Clutter::Behaviour::Opacity->new($alpha, 255, 0);
	$behaviour->apply($echo);
	$timeline->signal_connect('new-frame', sub {
		my ($timeline, $frame) = @_;
		print $frame, "\n";
	});
	$timeline->start();
	
	
	# Sets marker position on the map
	$marker->set_position(45.528178, -73.563788);
	
	return $marker;
}


sub create_static_circle {
	my ($marker) = @_;
	
	my $texture = Clutter::Texture::Cairo->new($MARKER_SIZE, $MARKER_SIZE);
	my $cr = $texture->create_context();
	
	# Draw the circle
	$cr->set_source_rgb(0, 0, 0);
	$cr->arc($MARKER_SIZE/2, $MARKER_SIZE/2, $MARKER_SIZE/2, 0, pi2);
	$cr->close_path();

	# Fill the circle
	$cr->set_source_rgba(0.1, 0.1, 0.9, 1.0);
	$cr->fill();
	
	# Add the circle to the marker
	$marker->add($texture);
	$texture->set_anchor_point_from_gravity('center');
	$texture->set_position(0, 0);
	
	return $texture;
}


sub create_echo_circle {
	my ($marker) = @_;
	
	my $texture = Clutter::Texture::Cairo->new($MARKER_SIZE * 2, $MARKER_SIZE * 2);
	my $cr = $texture->create_context();
	
	# Draw the circle
	$cr->set_source_rgb(0, 0, 0);
	$cr->arc($MARKER_SIZE, $MARKER_SIZE, $MARKER_SIZE * 0.9, 0, pi2);
	$cr->close_path();

	# Stroke the circle
	$cr->set_line_width(2.0);
	$cr->set_source_rgba(0.1, 0.1, 0.7, 1.0);
	$cr->stroke();

	# Add the circle to the marker
	$marker->add($texture);
	$texture->lower_bottom(); # Ensure it is under the previous circle
	$texture->set_position(0, 0);
	$texture->set_anchor_point_from_gravity('center');
	
	return $texture;
}
