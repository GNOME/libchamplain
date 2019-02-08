libchamplain 0.12 README
========================

libchamplain is a Clutter based Gtk widget to display rich, eye-candy and
interactive maps.

libchamplain requires:
  * glib >= 2.38
  * gtk >= 3.0
  * clutter >= 1.24
  * clutter-gtk >= 1.0
  * cairo >= 1.4
  * sqlite >= 3.0
  * libsoup >= 2.42

To build using autotools, run:
```
./autogen.sh; make; sudo make install
```

Alternatively, when using the meson build system, run:
```
meson _builddir; cd _builddir; ninja; sudo ninja install
```

The repository and bug report page is at:
* https://gitlab.gnome.org/GNOME/libchamplain

Release tarballs can be downloaded from:
* https://download.gnome.org/sources/libchamplain/

For a simple examples how to use the library, check the `demos` directory;
in particular, the `minimal-gtk.c` and `minimal.py` demos are good starting
points to see how to get the most basic map application running.

Full documentation can be found at:
* https://gnome.pages.gitlab.gnome.org/libchamplain/champlain
* https://gnome.pages.gitlab.gnome.org/libchamplain/champlain-gtk

The official mailing list is at:
* https://mail.gnome.org/mailman/listinfo/libchamplain-list

The official IRC channel is at:
* irc://irc.freenode.org/#champlain

---

libchamplain is licensed under the terms of the GNU Lesser General Public
License, version 2.1 or (at your option) later.
