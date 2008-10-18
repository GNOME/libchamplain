SUBDIRS = champlain-gtk demos docs/reference

pcfiles = champlain-gtk-@API_VERSION@.pc

champlain-gtk-@API_VERSION@.pc: champlain-gtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

CLEANFILES = $(pcfiles)
