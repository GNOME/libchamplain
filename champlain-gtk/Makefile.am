
BUILT_SOURCES = \
	champlain-gtk-marshal.h		 \
	champlain-gtk-marshal.c

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_GTK_MARSHAL = champlain-gtk-marshal
CHAMPLAIN_GTK_MARSHAL_LIST = champlain-gtk-marshal.list

lib_LTLIBRARIES = libchamplain-gtk-0.5.la

libchamplain_gtk_0_5_la_SOURCES = \
	$(CHAMPLAIN_GTK_MARSHAL_LIST)		 \
	$(BUILT_SOURCES) 			\
	gtk-champlain-embed.c

noinst_HEADERS = gtk-champlain-embed.h

libchamplain_gtk_0_5_la_LIBADD = $(GTK_DEPS_LIBS) 	\
	$(top_builddir)/champlain/libchamplain-0.5.la

champlain-gtk-marshal.h: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).h

champlain-gtk-marshal.c: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-gtk-0.5/champlain-gtk
libchamplain_include_HEADERS = champlain-gtk.h gtk-champlain-embed.h

libchamplain_gtk_0_5_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(GTK_DEPS_CFLAGS) -I$(top_srcdir) -DCHAMPLAIN_GTK_COMPILATION
AM_LDFLAGS = $(GTK_DEPS_LIBS) -export-symbols-regex ^gtk_champlain_.*

EXTRA_DIST = $(CHAMPLAIN_GTK_MARSHAL_LIST)

if HAVE_INTROSPECTION
BUILT_GIRSOURCES =

GtkChamplain-0.5.gir: $(INTROSPECTION_SCANNER) libchamplain-gtk-0.5.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
	--namespace GtkChamplain --nsversion=0.5 \
	--strip-prefix=GtkChamplain \
	$(GTK_DEPS_CFLAGS) \
	$(SOUP_CFLAGS) \
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	--add-include-path=$(builddir) \
	--c-include="$(libchamplain_include_HEADERS)" \
	--include=Champlain-0.4 \
	--include=Clutter-1.0 \
	--include=Gtk-2.0 \
	--library=champlain-gtk-0.5 \
	--libtool="$(LIBTOOL)" \
	--output $@ \
	--pkg gobject-2.0 \
	--pkg gtk+-2.0 \
	$(addprefix $(srcdir)/,$(libchamplain_gtk_0_5_la_SOURCES)) \
	$(addprefix $(srcdir)/,$(libchamplain_include_HEADERS))

BUILT_GIRSOURCES += GtkChamplain-0.5.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(QUIET_GEN)$(DEBUG) $(INTROSPECTION_COMPILER) --includedir=$(srcdir) --includedir=. $(INTROSPECTION_COMPILER_OPTS) $< -o $(builddir)/$(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif
