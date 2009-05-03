# -*- coding: utf-8 -*-
import clutter
import champlain

class DemoMarkerLayer(champlain.Layer):

	def __init__(self):
		champlain.Layer.__init__(self)

		orange = clutter.Color(0xf3, 0x94, 0x07, 0xbb)
		white = clutter.Color(0xff, 0xff, 0xff, 0xff)
		black = clutter.Color(0x00, 0x00, 0x00, 0xff)
		marker = champlain.marker_new_with_text("Montréal", "Airmole 14", black, orange)
		marker.set_position(45.528178, -73.563788)
		self.add(marker)

		marker = champlain.marker_new_with_text("New York", "Sans 25", white, orange);
		marker.set_position(40.77, -73.98);
		self.add(marker)

		marker = champlain.marker_new_with_text("Saint-Tite-des-Caps", "Serif 12", black, orange);
		marker.set_position(47.130885, -70.764141);
		self.add(marker)

		self.hide()
