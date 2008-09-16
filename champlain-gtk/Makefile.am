
BUILT_SOURCES = \
	champlain-gtk-marshal.h \
	champlain-gtk-marshal.c 

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_GTK_MARSHAL = champlain-gtk-marshal
CHAMPLAIN_GTK_MARSHAL_LIST = champlain-gtk-marshal.list 

lib_LTLIBRARIES = libchamplain-gtk-1.0.la

libchamplain_gtk_1_0_la_SOURCES = $(CHAMPLAIN_GTK_MARSHAL_LIST) \
					$(BUILT_SOURCES) \
					champlainviewembed.c

noinst_HEADERS = champlainviewembed.h

libchamplain_gtk_1_0_la_LIBADD = $(DEPS_LIBS)

champlain-gtk-marshal.h: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).h 

champlain-gtk-marshal.c: $(CHAMPLAIN_GTK_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_gtk_marshal $< > $(CHAMPLAIN_GTK_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-gtk-1.0/champlain-gtk
libchamplain_include_HEADERS = champlainviewembed.h

libchamplain_gtk_1_0_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

noinst_PROGRAMS = launcher

AM_CPPFLAGS = $(DEPS_CFLAGS) -I$(top_srcdir)
AM_LDFLAGS = $(DEPS_LIBS) -export-dynamic -export-symbols-regex ^champlain_.*
launcher_SOURCES = launcher.c
launcher_LDADD = $(DEPS_LIBS) libchamplain-gtk-1.0.la

EXTRA_DIST = $(CHAMPLAIN_GTK_MARSHAL_LIST)
