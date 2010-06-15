#!/usr/bin/env python
# -*- coding: utf-8 -*-
import gobject
import gtk

import champlaingtk # must be the first imported
import champlain
import clutter

def main():
    # initialize threads and clutter
    gobject.threads_init()
    clutter.init()

    # create the top-level window and quit the main loop when it's closed
    window = gtk.Window(gtk.WINDOW_TOPLEVEL)
    window.connect('destroy', gtk.main_quit)
    
    # create the libchamplain widget and set it's size
    widget = champlaingtk.ChamplainEmbed()
    widget.set_size_request(640, 480)
    
    view = widget.get_view()
    view.set_property('scroll-mode', champlain.SCROLL_MODE_KINETIC)
    view.set_property('zoom-level', 5)
    view.center_on(45.466, -73.75)
    
    # insert it into the widget you wish
    window.add(widget)

    # show everything    
    window.show_all()
    
    # start the main loop
    gtk.main()


if __name__ == '__main__':
    main()
