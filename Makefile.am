SUBDIRS = tidy champlain docs/reference

ACLOCAL_AMFLAGS = -I m4

pcfiles = champlain-@API_VERSION@.pc
champlain-@API_VERSION@.pc: champlain.pc
	@cp -f $< $@

if ENABLE_GTK
SUBDIRS += champlain-gtk docs/reference-gtk
pcfiles += champlain-gtk-@API_VERSION@.pc
champlain-gtk-@API_VERSION@.pc: champlain-gtk.pc
	@cp -f $< $@
endif


pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

CLEANFILES = $(pcfiles)

# Have the demos build at the end as they depend on optional parts
SUBDIRS += demos
