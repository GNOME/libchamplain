#!/usr/bin/perl

use strict;
use warnings;

use Glib qw(TRUE FALSE);
use Clutter qw(-gtk-init);
use Gtk2 qw(-init);
use Champlain;


exit main();


sub main {
	
	my $window = Gtk2::Window->new();
	$window->set_border_width(10);
	$window->set_title("Champlain - Demo");
	$window->signal_connect('destroy' => sub { Gtk2->main_quit() });
	
	my $vbox = Gtk2::VBox->new(FALSE, 10);	

	# Create the map view
	my $map = Champlain::View->new();
	my $gtk2_map = Gtk2::Champlain::ViewEmbed->new($map);
	$map->set('scroll-mode', 'kinetic', 'zoom-level', 5);
	$gtk2_map->set_size_request(640, 480);
	
	# Create the markers and marker layer
	my $layer = create_marker_layer($map);
	$map->add_layer($layer);


	##
	# Create the top toolbar
	my $toolbox = Gtk2::HBox->new(FALSE, 10);

	my $child = Gtk2::Button->new_from_stock('gtk-zoom-in');
	$child->signal_connect('clicked', sub {
		$map->zoom_in();	
	});
	$toolbox->add($child);

	$child = Gtk2::Button->new_from_stock('gtk-zoom-out');
	$child->signal_connect('clicked', sub {
		$map->zoom_out();	
	});
	$toolbox->add($child);

	$child = Gtk2::ToggleButton->new_with_label("Markers");
	$child->signal_connect('toggled', sub {
		if ($layer->get('visible')) {
			$layer->hide();		
		}
		else {
			$layer->show_all();		
		}
	});
	$toolbox->add($child);

	$child = Gtk2::ComboBox->new_text();
	my @sources = ();	
	foreach my $type qw(osm_mapnik oam mff_relief osm_cyclemap osm_osmarender) {
		my $constructor = "new_$type";
		my $source = 	Champlain::MapSource->$constructor();
		push @sources, $source;
		$child->append_text($source->get_name);
	}
	$child->set_active(0);
	$child->signal_connect('changed', sub {
		my ($button) = @_;		
		my $index = $button->get_active;
		my $source = $sources[$index];		
		$map->set('map-source', $source);
	});
	$toolbox->add($child);

	my $spin = Gtk2::SpinButton->new_with_range(0, 20, 1);
	$spin->signal_connect('changed', sub {
		$map->set('zoom-level', $spin->get_value_as_int);
	});
	$map->signal_connect('notify::zoom-level', sub {
		$spin->set_value($map->get('zoom-level'));
	});
	$toolbox->add($spin);

	my $image = Gtk2::Image->new_from_stock('gtk-network', 'button');
	$map->signal_connect('notify::state', sub {
		my $state = $map->get('state');
		if ($state eq 'loading') {
			$image->show();
		}
		else {
			$image->hide();		
		}
	});
	$toolbox->pack_end($image, FALSE, FALSE, 0);

	
	# Finish initializing the map view
	$map->center_on(45.466, -73.75);
	

	my $viewport = Gtk2::Viewport->new();
	$viewport->set_shadow_type('etched-in');
	$viewport->add($gtk2_map);

	$vbox->pack_start($toolbox, FALSE, FALSE, 0);
	$vbox->add($viewport);

	$window->add($vbox);
	$window->show_all();
	$image->hide();
	
	Gtk2->main();
	
	return 0;
}


sub create_marker_layer {
	my ($map) = @_;
	my $layer = Champlain::Layer->new();

	my $orange = Clutter::Color->new(0xf3, 0x94, 0x07, 0xbb);
	my $white = Clutter::Color->new(0xff, 0xff, 0xff, 0xff);
	
	my $marker;
	
	$marker = Champlain::Marker->new_with_label("Montr\x{e9}al", "Airmole 14", undef, undef);
	$marker->set_position(45.528178, -73.563788);
	$marker->set_reactive(TRUE);
	$marker->signal_connect_after("button-release-event", \&marker_button_release_cb, $map);
	$layer->add($marker);

	$marker = Champlain::Marker->new_with_label("New York", "Sans 15", $white, undef);
	$marker->set_position(40.77, -73.98);
	$layer->add($marker);

	$marker = Champlain::Marker->new_with_label("Bratislava", "Sans 15", $orange, undef);
	$marker->set_position(47.130885, -70.764141);
	$layer->add($marker);

	$layer->show();
	return $layer;
}


sub marker_button_release_cb {
	my ($marker, $event, $map) = @_;

	return FALSE if $event->button != 1 || $event->click_count > 1;
	
	print "Montreal was clicked\n";

	return TRUE;
}


sub map_view_button_release_cb {
	my ($actor, $event, $map) = @_;
	return FALSE if $event->button != 2 || $event->click_count > 1;

	my ($lat, $lon) = $map->get_coords_from_event($event);
	printf "Map was clicked at %f, %f\n", $lat, $lon;
	return TRUE;
}

