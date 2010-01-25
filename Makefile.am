SUBDIRS = tidy champlain docs/reference

ACLOCAL_AMFLAGS = -I m4

pcfiles = champlain-@API_VERSION@.pc
other_pcfiles = champlain-@API_VERSION@-uninstalled.pc
champlain-@API_VERSION@.pc: champlain.pc
	@cp -f $< $@
champlain-@API_VERSION@-uninstalled.pc: champlain-uninstalled.pc
	@cp -f $< $@

if ENABLE_GTK
SUBDIRS += champlain-gtk docs/reference-gtk
pcfiles += champlain-gtk-@API_VERSION@.pc
champlain-gtk-@API_VERSION@.pc: champlain-gtk.pc
	@cp -f $< $@
other_pcfiles += champlain-gtk-@API_VERSION@-uninstalled.pc
champlain-gtk-@API_VERSION@-uninstalled.pc: champlain-gtk-uninstalled.pc
	@cp -f $< $@
endif

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc --disable-python --disable-introspection

CLEANFILES = $(pcfiles) $(other_pcfiles)
all: $(other_pcfiles)

# Have the demos build at the end as they depend on optional parts
SUBDIRS += demos bindings

ChangeLog: Makefile.am
	git log --pretty=short > ChangeLog
