#!/usr/bin/env python
# -*- coding: utf-8 -*-
import gobject
import gtk
import clutter
import champlain

from twisted.internet import gtk2reactor
gtk2reactor.install()
import twisted.web.client as httpclient
from twisted.internet import reactor 

def texture_new_from_pixbuf(pixbuf):
    data = pixbuf.get_pixels()
    width = pixbuf.get_width()
    height = pixbuf.get_height()
    has_alpha = pixbuf.get_has_alpha()
    rowstride = pixbuf.get_rowstride()
    texture = clutter.Texture()
    success = texture.set_from_rgb_data(data, has_alpha, width, height, 
        rowstride, 4 if has_alpha else 3, clutter.TextureFlags(0))
    if not success:
        return None
    return texture


def pixbuf_new_from_data(data):
    loader = gtk.gdk.PixbufLoader()
    loader.write(data)
    loader.close()
    pixbuf = loader.get_pixbuf()
    return pixbuf


def image_download_cb(data, layer, latitude, longitude, url):
    if not data:
        return
    pixbuf = pixbuf_new_from_data(data)
    if not pixbuf:
        print "Failed to convert %s into an image" % url
    # Then transform the pixbuf into a texture
    texture = texture_new_from_pixbuf(pixbuf)
    if not texture:
        print "Failed to convert %s into an texture" % url
    # Finally create a marker with the texture
    marker = champlain.marker_new_with_image(texture)
    marker.set_position(latitude, longitude)
    layer.add(marker)
    marker.show_all()


def create_marker_from_url(layer, latitude, longitude, url):
    d = httpclient.getPage(url)
    d.addCallback(image_download_cb, layer, latitude, longitude, url)


def event_cb(stage, event):
    # use reactor.stop and avoid clutter.main_quit
    if event.type == clutter.DELETE:
        reactor.stop()
        return True


def main():
    gobject.threads_init()
    clutter.init()

    stage = clutter.Stage(default=True)
    stage.set_size(800, 600)
    stage.connect("event", event_cb)  

    # Create the map view
    view = champlain.View()
    view.set_size(800, 600)
    stage.add(view)

    # Create the markers and marker layer
    layer = champlain.Layer()
    view.add_layer(layer)
    create_marker_from_url(layer, 48.218611, 17.146397,
        "http://hexten.net/cpan-faces/potyl.jpg")
    create_marker_from_url(layer, 48.21066, 16.31476,
        "http://hexten.net/cpan-faces/jkutej.jpg")
    create_marker_from_url(layer, 48.14838, 17.10791,
        "http://bratislava.pm.org/images/whoiswho/jnthn.jpg")

    # Finish initialising the map view
    view.set_property('scroll-mode', champlain.SCROLL_MODE_KINETIC)
    view.set_property('zoom-level', 10)
    view.center_on(48.22, 16.8)  

    stage.show_all()
    reactor.run()


if __name__ == '__main__':
    main()
