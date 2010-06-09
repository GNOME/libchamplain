#!/usr/bin/env python
# -*- coding: utf-8 -*-
import gobject
import clutter
import champlain

PADDING = 10

def map_view_button_release_cb(actor, event, view):
    if event.button != 1 or event.click_count > 1:
        return True
    latlon = view.get_coords_from_event(event)
    if latlon:
        print "Map clicked at", latlon[0], latlon[1], "\n"
    return True

def zoom_in(actor, event, view):
    view.zoom_in()    
    return True

def zoom_out(actor, event, view):
    view.zoom_out()    
    return True

def make_button(text):
    button = clutter.Group()
    button_bg = clutter.Rectangle()
    button_bg.set_color(clutter.color_from_string('white'))
    button_bg.set_opacity(0xcc)
    button.add(button_bg)

    button_text = clutter.Text('Sans 10', text, \
        clutter.color_from_string('black'))
    button.add(button_text)

    width, height = button_text.get_size()
    button_bg.set_size(width+PADDING*2, height+PADDING*2)
    button_bg.set_position(0, 0)
    button_text.set_position(PADDING, PADDING)
    return button

def main():
    # Create the map view
    actor = champlain.View()
    actor.set_size(800, 600)
    actor.set_property('scroll-mode', champlain.SCROLL_MODE_KINETIC)
    actor.set_property('zoom-level', 9)

    # Create the buttons
    buttons = clutter.Group()
    buttons.set_position(PADDING, PADDING)
    
    button = make_button('Zoom in')
    button.set_reactive(True)
    width = button.get_size()[0]
    total_width = width+PADDING
    button.connect('button-release-event', zoom_in, actor)
    buttons.add(button)

    button = make_button('Zoom out')
    button.set_reactive(True)
    button.set_position(total_width, 0)
    width = button.get_size()[0]
    total_width += width+PADDING
    button.connect('button-release-event', zoom_out, actor)
    buttons.add(button)
    
    stage = clutter.Stage(default=True)
    stage.set_size(800, 600)
    stage.add(actor)
    stage.add(buttons)
    stage.show()

    # draw a line
    polygon = champlain.Polygon()
    # cheap approx of Highway 10
    polygon.append_point(45.4095, -73.3197)
    polygon.append_point(45.4104, -73.2846)
    polygon.append_point(45.4178, -73.2239)
    polygon.append_point(45.4176, -73.2181)
    polygon.append_point(45.4151, -73.2126)
    polygon.append_point(45.4016, -73.1926)
    polygon.append_point(45.3994, -73.1877)
    polygon.append_point(45.4000, -73.1815)
    polygon.append_point(45.4151, -73.1218)
    polygon.set_stroke_width(5.0)
    actor.add_polygon(polygon)

    # draw a polygon
    polygon = champlain.Polygon()
    polygon.append_point(45.1386, -73.9196)
    polygon.append_point(45.1229, -73.8991)
    polygon.append_point(45.0946, -73.9531)
    polygon.append_point(45.1085, -73.9714)
    polygon.append_point(45.1104, -73.9761)
    polygon.set_fill(True)
    polygon.set_property('closed-path', True)
    actor.add_polygon(polygon)

    # Connect to the click event
    actor.set_reactive(True)
    actor.connect('button-release-event', map_view_button_release_cb, actor)
    actor.center_on(45.466, -73.75)


if __name__ == "__main__":
    gobject.threads_init()
    clutter.init()
    main()
    clutter.main()
