
BUILT_SOURCES = \
	champlain-gtk-marshal.h		 \
	champlain-gtk-marshal.c

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_GTK_MARSHAL = champlain-gtk-marshal
CHAMPLAIN_GTK_MARSHAL_LIST = champlain-gtk-marshal.list

lib_LTLIBRARIES = libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	$(CHAMPLAIN_GTK_MARSHAL_LIST)		 \
	$(BUILT_SOURCES) 			\
	gtk-champlain-embed.c

noinst_HEADERS = gtk-champlain-embed.h

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_LIBADD = $(GTK_LIBS) 	\
	$(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

champlain-gtk-marshal.h: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).h

champlain-gtk-marshal.c: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-gtk-@CHAMPLAIN_API_VERSION@/champlain-gtk
libchamplain_include_HEADERS = champlain-gtk.h gtk-champlain-embed.h

libchamplain_gtk_@CHAMPLAIN_API_VERSION@_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(GTK_CFLAGS) -I$(top_srcdir) -DCHAMPLAIN_GTK_COMPILATION
AM_LDFLAGS = $(GTK_LIBS) -export-symbols-regex ^gtk_champlain_.*

EXTRA_DIST = $(CHAMPLAIN_GTK_MARSHAL_LIST)

if HAVE_INTROSPECTION
BUILT_GIRSOURCES =

GtkChamplain-@CHAMPLAIN_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
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
	$(QUIET_GEN)$(DEBUG) $(INTROSPECTION_COMPILER) --includedir=$(srcdir) --includedir=. --includedir=$(top_builddir)/champlain $(INTROSPECTION_COMPILER_OPTS) $< -o $(builddir)/$(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif
