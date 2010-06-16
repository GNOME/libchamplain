
BUILT_SOURCES = \
	champlain-gtk-marshal.h		 \
	champlain-gtk-marshal.c

CLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES = libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	champlain-gtk-marshal.list		 \
	$(BUILT_SOURCES) 			\
	gtk-champlain-embed.c

noinst_HEADERS = gtk-champlain-embed.h

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_LIBADD = $(GTK_LIBS) 	\
	$(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

champlain-gtk-marshal.h: champlain-gtk-marshal.list
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --header --prefix=champlain_gtk_marshal champlain-gtk-marshal.list > champlain-gtk-marshal.tmp \
	&& mv champlain-gtk-marshal.tmp champlain-gtk-marshal.h ) \
	|| ( rm -f champlain-gtk-marshal.tmp && exit 1 )

champlain-gtk-marshal.c: champlain-gtk-marshal.h
	$(AM_V_GEN) ( (echo '#include "champlain-gtk-marshal.h"'; $(GLIB_GENMARSHAL) --prefix=champlain_gtk_marshal $(srcdir)/champlain-gtk-marshal.list --body) > champlain-gtk-marshal.tmp \
	&& mv champlain-gtk-marshal.tmp champlain-gtk-marshal.c ) \
	|| ( rm -f champlain-gtk-marshal.tmp && exit 1 )

libchamplain_includedir=$(includedir)/libchamplain-gtk-@CHAMPLAIN_API_VERSION@/champlain-gtk
libchamplain_include_HEADERS = champlain-gtk.h gtk-champlain-embed.h

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(GTK_CFLAGS) -I$(top_srcdir) -DCHAMPLAIN_GTK_COMPILATION
AM_LDFLAGS = $(GTK_LIBS) -export-symbols-regex ^gtk_champlain_.*

EXTRA_DIST = champlain-gtk-marshal.list

if HAVE_INTROSPECTION

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(top_builddir)/champlain
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=$(top_builddir)/champlain 

introspection_sources = \
	$(libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES) \
	$(libchamplain_include_HEADERS)

GtkChamplain-@CHAMPLAIN_API_VERSION@.gir: libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_NAMESPACE = GtkChamplain
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_VERSION = @CHAMPLAIN_API_VERSION@
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_LIBS = libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_PACKAGES = gobject-2.0 gtk+-2.0
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_SCANNERFLAGS = --strip-prefix=GtkChamplain
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_INCLUDES = Clutter-1.0 Gtk-2.0 Champlain-@CHAMPLAIN_API_VERSION@
GtkChamplain_@CHAMPLAIN_API_VERSION_NORM@_gir_CFLAGS = \
	$(DEPS_CFLAGS) $(SOUP_CFLAGS) $(GTK_CFLAGS) \
	-I$(top_srcdir) \
	-DCHAMPLAIN_COMPILATION -DCHAMPLAIN_GTK_COMPILATION

INTROSPECTION_GIRS += GtkChamplain-@CHAMPLAIN_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif
