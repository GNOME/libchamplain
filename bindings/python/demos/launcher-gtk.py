#!/usr/bin/env python
# -*- coding: utf-8 -*-
import clutter
import cluttergtk
import gobject
import gtk
import champlain
import champlaingtk

class champlainDemo:
	def zoom_in(widget, view):
		view.zoom_in()

	def zoom_out(widget, view):
		view.zoom_out()

	def toggle_layer(widget, layer):
		if widget.get_active():
			layer.show_all()
		else:
			layer.hide()

	def zoom_changed(spinbutton, view):
		view.set_property("zoom-level", spinbutton.get_value_as_int())

	def map_source_changed(widget, view):
		selection = widget.get_active_text()
		if selection == "Open Street Map":
			view.set_property("map-source", champlain.map_source_new_osm_mapnik())
		elif selection == "Open Arial Map":
			view.set_property("map-source", champlain.map_source_new_oam())
		elif selection == "Maps for free - Relief":
			view.set_property("map-source", champlain.map_source_new_mff_relief())
		elif selection == "OSM Cycle Map":
			view.set_property("map-source", champlain.map_source_new_osm_cyclemap())
		elif selection == "OSM Osmarender":
			view.set_property("map-source", champlain.map_source_new_osm_osmarender())
		else:
			raise RuntimeException("Illegal state: active text of combobox invalid")

	def map_zoom_changed(view, obj, spinbutton):
		spinbutton.set_value(view.get_property("zoom-level"))

	def create_marker_layer():
		layer = champlain.Layer()

		orange = clutter.Color(0xf3, 0x94, 0x07, 0xbb)
		white = clutter.Color(0xff, 0xff, 0xff, 0xff)
		black = clutter.Color(0x00, 0x00, 0x00, 0xff)
		marker = champlain.marker_new_with_text("Montréal", "Airmole 14", black, orange)
		marker.set_position(45.528178, -73.563788)
		layer.add(marker)

		marker = champlain.marker_new_with_text("New York", "Sans 25", white, orange);
		marker.set_position(40.77, -73.98);
		layer.add(marker)

		marker = champlain.marker_new_with_text("Saint-Tite-des-Caps", "Serif 12", black, orange);
		marker.set_position(47.130885, -70.764141);
		layer.add(marker)

		layer.hide()

		return layer

	gobject.threads_init()
	clutter.init()

	window = gtk.Window()
	window.set_border_width(10)
	window.set_title("The world seen through the eyes of a Python")

	window.connect("destroy", lambda w: gtk.main_quit)

	vbox = gtk.VBox(False, 12)

	view = champlain.View()
	view.set_property("scroll-mode", champlain.SCROLL_MODE_KINETIC)
	view.set_property("zoom-level", 5)

	layer = create_marker_layer()
	view.add_layer(layer)

	embed = cluttergtk.Embed()
	embed.set_size_request(640, 480)

	bbox = gtk.HBox(False, 6)
	button = gtk.Button(stock=gtk.STOCK_ZOOM_IN)
	button.connect("clicked", zoom_in, view)
	bbox.add(button)

	button = gtk.Button(stock=gtk.STOCK_ZOOM_OUT)
	button.connect("clicked", zoom_out, view)
	bbox.add(button)

	button = gtk.ToggleButton(label="Markers")
	button.connect("toggled", toggle_layer, layer)
	bbox.add(button)

	combo = gtk.combo_box_new_text()
	combo.append_text("Open Street Map")
	combo.append_text("Open Arial Map")
	combo.append_text("Maps for free - Relief")
	combo.append_text("OSM Cycle Map")
	combo.append_text("OSM Osmarender")
	combo.set_active(0)
	combo.connect("changed", map_source_changed, view)
	bbox.add(combo)

	button = gtk.SpinButton(gtk.Adjustment(lower=0, upper=20, value=1, step_incr=1))
	button.connect("changed", zoom_changed, view)
	view.connect("notify::zoom-level", map_zoom_changed, button)
	bbox.add(button)

	vbox.pack_start(bbox, expand=False, fill=False)
	vbox.add(embed)

	window.add(vbox)
	# we need to realize the widget before we get the stage
	embed.realize()

	stage = embed.get_stage()
	view.set_size(640, 480)
	stage.add(view)

	window.show_all()
	view.center_on(45.466, -73.75)

def main():
    gtk.main()
    return 0

if __name__ == "__main__":
    champlainDemo()
    main()
