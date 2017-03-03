BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =


libchamplain_gtk_headers_public =	\
	$(srcdir)/gtk-champlain-embed.h	\
	$(srcdir)/champlain-gtk.h

libchamplain_gtk_sources = \
	gtk-champlain-embed.c


# glib-genmarshal rules
glib_marshal_list = champlain-gtk-marshal.list
glib_marshal_prefix = _champlain_gtk_marshal
include $(top_srcdir)/build/Makefile.am.marshal

# glib-mkenums rules
glib_enum_h = champlain-gtk-enum-types.h
glib_enum_c = champlain-gtk-enum-types.c
glib_enum_headers = $(libchamplain_gtk_headers_public)
include $(top_srcdir)/build/Makefile.am.enums

libchamplain_gtk_headers_built =	\
	champlain-gtk-enum-types.h		\
	champlain-gtk-marshal.h

libchamplain_gtk_sources_built =	\
	champlain-gtk-enum-types.c		\
	champlain-gtk-marshal.c


libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	$(libchamplain_gtk_headers_public)	\
	$(libchamplain_gtk_sources)

nodist_libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	$(libchamplain_gtk_headers_built)	\
	$(libchamplain_gtk_sources_built)

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_LIBADD = \
	$(GTK_LIBS) 	\
	$(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_LDFLAGS= \
	-version-info $(LIBRARY_VERSION) \
	-no-undefined \
	-export-symbols-regex ^gtk_champlain_.*

AM_CPPFLAGS = $(DEPS_CFLAGS) $(GTK_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir) -DCHAMPLAIN_GTK_COMPILATION


lib_LTLIBRARIES = libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la

libchamplain_gtkdir=$(includedir)/libchamplain-gtk-@CHAMPLAIN_API_VERSION@/champlain-gtk

libchamplain_gtk_HEADERS = $(libchamplain_gtk_headers_public)

nodist_libchamplain_gtk_HEADERS = $(libchamplain_gtk_headers_built)

# Make the g-ir-scanner to find the not yet installed pkg-config files.
PKG_CONFIG_PATH := $(top_builddir):$(PKG_CONFIG_PATH)

if HAVE_INTROSPECTION

-include $(INTROSPECTION_MAKEFILE)

INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all --add-include-path=$(top_builddir)/champlain
INTROSPECTION_COMPILER_ARGS = --includedir=$(top_builddir)/champlain

introspection_sources = $(libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES)

GtkChamplain-@CHAMPLAIN_API_VERSION@.gir: libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_NAMESPACE = GtkChamplain
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_VERSION = @CHAMPLAIN_API_VERSION@
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_LIBS = libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_PACKAGES = gobject-2.0 gtk+-3.0 pangocairo
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_FILES = $(introspection_sources)
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_INCLUDES = Clutter-1.0 Gtk-3.0
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_EXPORT_PACKAGES = champlain-gtk-@CHAMPLAIN_API_VERSION@
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_SCANNERFLAGS = \
	--library-path=$(top_builddir)/champlain \
	--identifier-prefix=GtkChamplain \
	--symbol-prefix=gtk_champlain \
	--c-include="champlain-gtk/champlain-gtk.h" \
	--include-uninstalled=$(top_builddir)/champlain/Champlain-@CHAMPLAIN_API_VERSION@.gir
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_CFLAGS = \
	$(GTK_CFLAGS) $(DEPS_CFLAGS) \
	-I$(top_srcdir) -I$(top_builddir)\
	-DCHAMPLAIN_COMPILATION -DCHAMPLAIN_GTK_COMPILATION

INTROSPECTION_GIRS += GtkChamplain-@CHAMPLAIN_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif

vapidir = $(datadir)/vala/vapi
dist_vapi_DATA = champlain-gtk-@CHAMPLAIN_API_VERSION@.vapi
EXTRA_DIST += champlain-gtk-@CHAMPLAIN_API_VERSION@.vapi

if ENABLE_VALA

CLEANFILES += $(dist_vapi_DATA)

champlain-gtk-@CHAMPLAIN_API_VERSION@.vapi:	GtkChamplain-@CHAMPLAIN_API_VERSION@.gir
	$(AM_V_GEN) ( $(VAPIGEN) --library champlain-gtk-@CHAMPLAIN_API_VERSION@ --vapidir=$(top_builddir)/champlain --pkg champlain-@CHAMPLAIN_API_VERSION@ --pkg clutter-gtk-1.0 --pkg cogl-pango-1.0 --pkg gtk+-3.0 --pkg atk --pkg pangocairo --girdir=$(top_builddir)/champlain $(builddir)/GtkChamplain-@CHAMPLAIN_API_VERSION@.gir )

endif

