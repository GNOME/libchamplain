
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
BUILT_GIRSOURCES =

GtkChamplain-@CHAMPLAIN_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la
	$(AM_V_GEN)$(INTROSPECTION_SCANNER) -v \
	--namespace GtkChamplain --nsversion=@CHAMPLAIN_API_VERSION@ \
	--strip-prefix=GtkChamplain \
	$(GTK_CFLAGS) \
	$(SOUP_CFLAGS) \
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	-DCHAMPLAIN_GTK_COMPILATION 	\
	--add-include-path=$(top_builddir)/champlain \
	--c-include="$(libchamplain_include_HEADERS)" \
	--include=Champlain-0.5 \
	--include=Clutter-1.0 \
	--include=Gtk-2.0 \
	--library=champlain-gtk-@CHAMPLAIN_API_VERSION@ \
	--libtool="$(LIBTOOL)" \
	--output $@ \
	--pkg gobject-2.0 \
	--pkg gtk+-2.0 \
	$(addprefix $(srcdir)/,$(libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES)) \
	$(addprefix $(srcdir)/,$(libchamplain_include_HEADERS))

BUILT_GIRSOURCES += GtkChamplain-@CHAMPLAIN_API_VERSION@.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

GtkChamplain-@CHAMPLAIN_API_VERSION@.typelib: GtkChamplain-@CHAMPLAIN_API_VERSION@.gir $(INTROSPECTION_COMPILER)
	$(AM_V_GEN)$(DEBUG) $(INTROSPECTION_COMPILER) --includedir=$(srcdir) --includedir=. --includedir=$(top_builddir)/champlain $(INTROSPECTION_COMPILER_OPTS) $< -o $(builddir)/$(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif
