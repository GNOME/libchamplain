# -*- coding: utf-8 -*-
from gi.repository import Clutter
from gi.repository import Champlain
from gi.repository import Pango

def marker_button_release_cb(actor, event, view):
    if event.button != 1 and event.click_count > 1:
        return False

    print "Montreal was clicked\n"
    return True


def create_marker_layer(view):
    orange = Clutter.Color.new(0xf3, 0x94, 0x07, 0xbb)
    layer = Champlain.MarkerLayer()

    marker = Champlain.Label.new_with_text(
        "Montréal\n<span size=\"xx-small\">Québec</span>", "Serif 14", None,
        orange)
    marker.set_use_markup(True)
    marker.set_alignment(Pango.Alignment.RIGHT)
    marker.set_color(orange)

    marker.set_location(45.528178, -73.563788)
    layer.add_marker(marker)
    marker.set_reactive(True)
    marker.connect("button-release-event", marker_button_release_cb, view)

    marker = Champlain.Label.new_from_file(
        "icons/emblem-generic.png")
    marker.set_text("New York")
    marker.set_location(40.77, -73.98)
    layer.add_marker(marker)

    marker = Champlain.Label.new_from_file(
        "icons/emblem-important.png")
    marker.set_location(47.130885, -70.764141)
    layer.add_marker(marker)

    marker = Champlain.Label.new_from_file(
        "icons/emblem-favorite.png")
    marker.set_draw_background(False)
    marker.set_location(45.41484, -71.918907)
    layer.add_marker(marker)

    marker = Champlain.Label.new_from_file(
        "icons/emblem-new.png")
    marker.set_draw_background(False)
    marker.set_location(50.639663, 5.570798)
    layer.add_marker(marker)

    layer.set_all_markers_draggable()
    layer.show()
    return layer
