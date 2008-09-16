SUBDIRS = champlain-gtk docs/reference

pcfiles = champlain-gtk-1.0.pc

champlain-gtk-1.0.pc: champlain-gtk.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

