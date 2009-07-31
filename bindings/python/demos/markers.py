#!/usr/bin/env python
# -*- coding: utf-8 -*-
import clutter
import champlain
import pango

def marker_button_release_cb(actor, event, view):
    if event.button != 1 and event.click_count > 1:
        return False

    print "Montreal was clicked\n" 
    return True


def create_marker_layer(view):
    orange = clutter.Color(0xf3, 0x94, 0x07, 0xbb)
    black = clutter.Color(0x00, 0x00, 0x00, 0xff)
    layer = champlain.Layer()

    marker = champlain.marker_new_with_text(
        "Montréal\n<span size=\"xx-small\">Québec</span>", "Serif 14", black, 
        orange)
    marker.set_use_markup(True)
    marker.set_alignment(pango.ALIGN_RIGHT)
    marker.set_color(orange)

    marker.set_position(45.528178, -73.563788)
    layer.add_marker(marker)
    marker.set_reactive(True)
    marker.connect("button-release-event", marker_button_release_cb, view)

    marker = champlain.marker_new_from_file(
        "/usr/share/icons/gnome/24x24/emblems/emblem-generic.png")
    marker.set_text("New York")
    marker.set_position(40.77, -73.98)
    layer.add_marker(marker)

    marker = champlain.marker_new_from_file(
        "/usr/share/icons/gnome/24x24/emblems/emblem-important.png")
    marker.set_position(47.130885, -70.764141)
    layer.add_marker(marker)

    marker = champlain.marker_new_from_file(
        "/usr/share/icons/gnome/24x24/emblems/emblem-favorite.png")
    marker.set_draw_background(False)
    marker.set_position(45.41484, -71.918907)
    layer.add_marker(marker)

    layer.show()
    return layer
