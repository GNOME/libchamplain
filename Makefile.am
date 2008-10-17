SUBDIRS = champlain-gtk demos docs/reference

pcfiles = champlain-gtk-0.2.pc

champlain-gtk-0.2.pc: champlain-gtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

CLEANFILES = $(pcfiles)
