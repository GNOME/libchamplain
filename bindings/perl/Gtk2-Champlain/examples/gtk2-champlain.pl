#!/usr/bin/perl

use strict;
use warnings;

use Glib qw(TRUE FALSE);
use Gtk2::Clutter '-init';
use Gtk2::SimpleList;
use Gtk2::Champlain;


exit main();


sub main {
	
	my $window = Gtk2::Window->new();
	$window->set_border_width(10);
	$window->set_title("Champlain - Demo");
	$window->signal_connect('destroy' => sub { Gtk2->main_quit() });
	
	my $vbox = Gtk2::VBox->new(FALSE, 10);	

	# Create the map view
	my $gtk2_map = Gtk2::Champlain::Embed->new();
	my $map = $gtk2_map->get_view();
	$map->set_scroll_mode('kinetic');
	$map->set_zoom_level(5);
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

	$child = Gtk2::Button->new_from_stock('gtk-home');
	$child->signal_connect('clicked', sub {
		$map->go_to(48.218611, 17.146397);
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
	
	$toolbox->add(create_combo_box($map));

	my $spin = Gtk2::SpinButton->new_with_range(0, 20, 1);
	$spin->set_value($map->get_zoom_level);
	$spin->signal_connect('changed', sub {
		$map->set_zoom_level($spin->get_value_as_int);
	});
	$map->signal_connect('notify::zoom-level', sub {
		$spin->set_value($map->get_zoom_level);
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


sub create_combo_box {
	my ($map) = @_;
	

	# Create a simple list that will be used as the data model of the ComboBox
	my $model = Gtk2::ListStore->new(
		'Glib::String',
		'Glib::String',
	);
	my $active = 0; # Tells which map source is active
	my $index = 0;
	my $current_source = $map->get_map_source->get_id;
	my $factory = Champlain::MapSourceFactory->dup_default;
	foreach my $desc (sort { $a->name cmp $b->name } $factory->dup_list) {
		my $iter = $model->append();
		$model->set($iter, 
			0, $desc->name,
			1, $desc->id,
		);
		
		if ($current_source eq $desc->id) {
			$active = $index;
		}
		
		++$index;
	}

	my $combo = Gtk2::ComboBox->new_text();
	$combo->set_model($model);
	$combo->set_active($active);
	
	
	$combo->signal_connect('changed', sub {
		my ($button) = @_;

		# Get the ID of the map source selected
		my $iter = $button->get_active_iter;
		my $id = $model->get($iter, 1);
		
		# Create that map source
		my $source = $factory->create($id);
		$map->set_map_source($source);
	});
	
	return $combo;
}


sub create_marker_layer {
	my ($map) = @_;
	my $layer = Champlain::Layer->new();

	my $orange = Clutter::Color->new(0xf3, 0x94, 0x07, 0xbb);
	my $white = Clutter::Color->new(0xff, 0xff, 0xff, 0xff);
	
	my $marker;
	
	$marker = Champlain::Marker->new_with_text("Montr\x{e9}al", "Airmole 14");
	$marker->set_position(45.528178, -73.563788);
	$marker->set_reactive(TRUE);
	$marker->signal_connect_after("button-release-event", \&marker_button_release_cb, $map);
	$layer->add($marker);

	$marker = Champlain::Marker->new_with_text("New York", "Sans 15", $white);
	$marker->set_position(40.77, -73.98);
	$layer->add($marker);

	$marker = Champlain::Marker->new_with_text("Bratislava", "Sans 15", $orange);
	$marker->set_position(48.148377, 17.107311);
	$layer->add($marker);

	$layer->show();
	return $layer;
}


sub marker_button_release_cb {
	my ($marker, $event, $map) = @_;
	return FALSE unless $event->button == 1 && $event->click_count == 1;

	print "Montreal was clicked\n";
	return TRUE;
}


sub map_view_button_release_cb {
	my ($actor, $event, $map) = @_;
	return FALSE unless $event->button == 2 && $event->click_count == 1;

	my ($lat, $lon) = $map->get_coords_from_event($event);
	printf "Map was clicked at %f, %f\n", $lat, $lon;
	return TRUE;
}
