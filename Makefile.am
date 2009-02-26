SUBDIRS = tidy champlain demos docs/reference

ACLOCAL_AMFLAGS = -I m4

pcfiles = champlain-@API_VERSION@.pc

champlain-@API_VERSION@.pc: champlain.pc
	@cp -f $< $@

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc

CLEANFILES = $(pcfiles)

ChangeLog: Makefile.am
	git-log --pretty=short > ChangeLog
	

