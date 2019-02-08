SUBDIRS = build champlain

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

pcfiles = champlain-@CHAMPLAIN_API_VERSION@.pc
other_pcfiles = champlain-@CHAMPLAIN_API_VERSION@-uninstalled.pc

if ENABLE_GTK
SUBDIRS += champlain-gtk
pcfiles += champlain-gtk-@CHAMPLAIN_API_VERSION@.pc
other_pcfiles += champlain-gtk-@CHAMPLAIN_API_VERSION@-uninstalled.pc
endif

pkgconfig_DATA = $(pcfiles)
pkgconfigdir = $(libdir)/pkgconfig

DISTCHECK_CONFIGURE_FLAGS=--enable-gtk-doc --enable-gtk --enable-introspection --enable-vala-demos --disable-memphis

EXTRA_DIST = \
	README.md \
	champlain-gtk.pc.in \
	champlain-gtk-uninstalled.pc.in \
	champlain.pc.in \
	champlain-uninstalled.pc.in

DISTCLEANFILES = $(pcfiles) $(other_pcfiles)

all: $(other_pcfiles)

# Have the demos build at the end as they depend on optional parts
SUBDIRS += docs demos
