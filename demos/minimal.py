#!/usr/bin/env python

from gi.repository import GtkClutter
GtkClutter.init([])
from gi.repository import GObject, Gtk, GtkChamplain 

GObject.threads_init()
GtkClutter.init([])

window = Gtk.Window(type=Gtk.WindowType.TOPLEVEL)
window.connect("destroy", Gtk.main_quit)

widget = GtkChamplain.Embed()
widget.set_size_request(640, 480)

window.add(widget)
window.show_all()

Gtk.main()

