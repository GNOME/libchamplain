SUBDIRS = tidy champlain champlain-gtk demos docs/reference

ACLOCAL_AMFLAGS = -I m4

pcfiles = champlain-@API_VERSION@.pc champlain-gtk-@API_VERSION@.pc

champlain-@API_VERSION@.pc: champlain.pc
	@cp -f $< $@

champlain-gtk-@API_VERSION@.pc: champlain-gtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

CLEANFILES = $(pcfiles)
