#!/usr/bin/env python
# -*- coding: utf-8 -*-
import cluttergtk # must be the first to be imported
import clutter
import gobject
import gtk
import champlain
import champlaingtk

from markers import create_marker_layer

class LauncherGTK:
    def __init__(self):
        self.window = gtk.Window()
        self.window.set_border_width(10)
        self.window.set_title("The world seen through the eyes of a Python")
        self.window.connect("destroy", gtk.main_quit)

        vbox = gtk.VBox(False, 12)

        self.view = champlain.View()
        self.view.set_property("scroll-mode", champlain.SCROLL_MODE_KINETIC)

        self.layer = create_marker_layer(self.view)
        self.view.add_layer(self.layer)

        self.polygon = polygon = champlain.Polygon()
        # Cheap approx of Highway 10
        polygon.append_point(45.4095, -73.3197)
        polygon.append_point(45.4104, -73.2846)
        polygon.append_point(45.4178, -73.2239)
        polygon.append_point(45.4176, -73.2181)
        polygon.append_point(45.4151, -73.2126)
        polygon.append_point(45.4016, -73.1926)
        polygon.append_point(45.3994, -73.1877)
        polygon.append_point(45.4000, -73.1815)
        polygon.append_point(45.4151, -73.1218)
        polygon.set_stroke_width(5.0);
        polygon.set_property("mark-points", True)
        self.view.add_polygon(polygon)
        polygon.hide()

        embed = cluttergtk.Embed()
        embed.set_size_request(640, 480)

        bbox = gtk.HBox(False, 6)
        button = gtk.Button(stock=gtk.STOCK_ZOOM_IN)
        button.connect("clicked", self.zoom_in)
        bbox.add(button)

        button = gtk.Button(stock=gtk.STOCK_ZOOM_OUT)
        button.connect("clicked", self.zoom_out)
        bbox.add(button)

        button = gtk.ToggleButton(label="Markers")
        button.connect("toggled", self.toggle_layer)
        bbox.add(button)

        combo = gtk.combo_box_new_text()
        self.map_source_factory = champlain.map_source_factory_dup_default()
        liststore = gtk.ListStore(str, str)
        for source in self.map_source_factory.dup_list():
            liststore.append([source.id, source.name])
            combo.append_text(source.name)
        combo.set_model(liststore)
        combo.set_attributes(combo.get_cells()[0], text=1)
        combo.set_active(0)
        combo.connect("changed", self.map_source_changed)
        bbox.add(combo)

        self.spinbutton = gtk.SpinButton(gtk.Adjustment(lower=0, upper=20, 
            value=1, step_incr=1))
        self.spinbutton.connect("changed", self.zoom_changed)
        self.view.connect("notify::zoom-level", self.map_zoom_changed)
        self.spinbutton.set_value(5)
        bbox.add(self.spinbutton)

        vbox.pack_start(bbox, expand=False, fill=False)
        vbox.add(embed)

        self.window.add(vbox)
        # we need to realize the widget before we get the stage
        embed.realize()

        stage = embed.get_stage()
        self.view.set_size(640, 480)
        stage.add(self.view)

        self.window.show_all()
        self.view.center_on(45.466, -73.75)

    def zoom_in(self, widget):
        self.view.zoom_in()

    def zoom_out(self, widget):
        self.view.zoom_out()

    def toggle_layer(self, widget):
        if widget.get_active():
            self.polygon.show()    
            self.layer.animate_in_all_markers()
        else:
            self.polygon.hide()    
            self.layer.animate_out_all_markers()

    def zoom_changed(self, widget):
        self.view.set_property("zoom-level", self.spinbutton.get_value_as_int())

    def map_source_changed(self, widget):
        model = widget.get_model()
        iter = widget.get_active_iter()
        id = model.get_value(iter, 0)
        source = self.map_source_factory.create(id);
        self.view.set_property("map-source", source)

    def map_zoom_changed(self, widget, value):
        self.spinbutton.set_value(self.view.get_property("zoom-level"))


if __name__ == "__main__":
    gobject.threads_init()
    LauncherGTK()
    gtk.main()
