SUBDIRS = tidy champlain demos docs/reference

pcfiles = champlain-@API_VERSION@.pc

champlain-@API_VERSION@.pc: champlain.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

CLEANFILES = $(pcfiles)

