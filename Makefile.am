SUBDIRS = tidy champlain docs/reference

ACLOCAL_AMFLAGS = -I m4

pcfiles = champlain-@CHAMPLAIN_API_VERSION@.pc
other_pcfiles = champlain-@CHAMPLAIN_API_VERSION@-uninstalled.pc
champlain-@CHAMPLAIN_API_VERSION@.pc: champlain.pc
	@cp -f $< $@
champlain-@CHAMPLAIN_API_VERSION@-uninstalled.pc: champlain-uninstalled.pc
	@cp -f $< $@

if ENABLE_GTK
SUBDIRS += champlain-gtk docs/reference-gtk
pcfiles += champlain-gtk-@CHAMPLAIN_API_VERSION@.pc
champlain-gtk-@CHAMPLAIN_API_VERSION@.pc: champlain-gtk.pc
	@cp -f $< $@
other_pcfiles += champlain-gtk-@CHAMPLAIN_API_VERSION@-uninstalled.pc
champlain-gtk-@CHAMPLAIN_API_VERSION@-uninstalled.pc: champlain-gtk-uninstalled.pc
	@cp -f $< $@
endif

if ENABLE_MEMPHIS
pcfiles += champlain-memphis-@CHAMPLAIN_API_VERSION@.pc
champlain-memphis-@CHAMPLAIN_API_VERSION@.pc: champlain-memphis.pc
	@cp -f $< $@
other_pcfiles += champlain-memphis-@CHAMPLAIN_API_VERSION@-uninstalled.pc
champlain-memphis-@CHAMPLAIN_API_VERSION@-uninstalled.pc: champlain-memphis-uninstalled.pc
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
