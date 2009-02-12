
BUILT_SOURCES = \
	champlain-gtk-marshal.h		 \
	champlain-gtk-marshal.c

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_GTK_MARSHAL = champlain-gtk-marshal
CHAMPLAIN_GTK_MARSHAL_LIST = champlain-gtk-marshal.list

lib_LTLIBRARIES = libchamplain-gtk-0.3.la

libchamplain_gtk_0_3_la_SOURCES = \
	$(CHAMPLAIN_GTK_MARSHAL_LIST)		 \
	$(BUILT_SOURCES) 			\
	champlain-view-embed.c

noinst_HEADERS = champlain-view-embed.h

libchamplain_gtk_0_3_la_LIBADD = $(GTK_DEPS_LIBS)

champlain-gtk-marshal.h: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).h 

champlain-gtk-marshal.c: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-gtk-0.3/champlain-gtk
libchamplain_include_HEADERS = champlain-gtk.h champlain-view-embed.h

libchamplain_gtk_0_3_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(GTK_DEPS_CFLAGS) -I$(top_srcdir) -DCHAMPLAIN_GTK_COMPILATION
AM_LDFLAGS = $(GTK_DEPS_LIBS) -export-symbols-regex ^champlain_.*

EXTRA_DIST = $(CHAMPLAIN_GTK_MARSHAL_LIST)
