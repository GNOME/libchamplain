SUBDIRS = tidy champlain demos docs/reference

pcfiles = champlain-0.2.pc

champlain-0.2.pc: champlain.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

