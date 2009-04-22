#!/usr/bin/env python

import clutter, cluttergtk
import champlain
import gtk, gobject
from twisted.internet import gtk2reactor
gtk2reactor.install()
from twisted.internet import reactor
import twisted.web.client as httpclient
from BeautifulSoup import BeautifulSoup


BASE_URL = "http://en.wikipedia.org"
CAPITALS_URL = "%s/wiki/List_of_national_capitals" % BASE_URL

class Capitals:


	def __init__(self):
		self.win = gtk.Window()
		self.win.set_title("A Capital Idea")

		self.capital_uris = []
		self.markers = []
	
		# Create the map stuff
		self.map = champlain.View()
		self.embed = cluttergtk.Embed()
		self.embed.set_size_request(640, 480)

		self.map.set_property("scroll-mode", champlain.SCROLL_MODE_KINETIC)
		self.map.set_property("zoom-level", 3)

		self.layer = champlain.Layer()
		self.map.add_layer(self.layer)

		self.win.add(self.embed)
		self.win.connect("destroy", lambda w: reactor.stop())

		self.embed.realize()
		stage = self.embed.get_stage()
		self.map.set_size(640, 480)
		stage.add(self.map)

		self.win.show_all()
		self.map.center_on(0, 0)

		# Download the next map after the go-to animation has been completed
		self.map.connect('animation-completed::go-to', lambda w: gobject.timeout_add_seconds(1, self.download_capital))
	
		d = httpclient.getPage(CAPITALS_URL)
		d.addCallback(self.capitals_main_cb)


	def capitals_main_cb(self, data):
		"""
		Called when the main page with all the capitals is downloaded.
		"""

		soup = BeautifulSoup(data)
		table = soup.find("table", { "class" : "wikitable sortable" })
		for row in table.findAll("tr"):
			cell = row.find("td")
			if cell:
				link = cell.find("a")
				uri = str(link['href'])
				self.capital_uris.append(uri)

		self.download_capital()


	def capital_cb(self, data):
		"""
		Called when the page of a capital is downloaded. The page is expected to have
		the coordinates of the capital.
		"""

		soup = BeautifulSoup(data)
		heading = soup.find("h1", { "id" : "firstHeading" })
		if not heading:
			return
		name = heading.contents[0]

		geo = soup.find("span", { "class" : "geo" })
		if not geo:
			return
		latitude, longitude = geo.contents[0].split(";")
		latitude = float(latitude)
		longitude = float(longitude)

		# Keep only a few capitals at each iteration we remove a capital
		if len(self.markers) == 5:
			marker = self.markers[0]
			self.layer.remove(marker)
			self.markers.remove(marker)

		font = "Sans 15"
		white = clutter.Color(0xff, 0xff, 0xff, 0xff)
		orange = clutter.Color(0xf3, 0x94, 0x07, 0xbb)
		black = clutter.Color(0x00, 0x00, 0x00, 0xff)

		if self.markers:
			#Change the colour of the last marker's text
			last = self.markers.pop()
			self.layer.remove(last)
			marker = champlain.marker_new_with_text(last.name, font, black, orange)
			marker.set_position(last.get_property("latitude"), last.get_property("longitude"))
			self.markers.append(marker)
			self.layer.add(marker)
			marker.raise_top()

		marker = champlain.marker_new_with_text(name, font, white, orange)
		marker.set_position(latitude, longitude)
		marker.name = name
		self.markers.append(marker)
		self.layer.add(marker)
		marker.raise_top()
		self.map.go_to(latitude, longitude)


	def download_capital(self):
		if not self.capital_uris:
			# No more capitals to display
			return
		uri = self.capital_uris[0]
		self.capital_uris.remove(uri)
		d = httpclient.getPage(BASE_URL + uri)
		d.addCallback(self.capital_cb)


if __name__ == "__main__":
	gobject.threads_init()
	clutter.init()
	Capitals()
	reactor.run()
