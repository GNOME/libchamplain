
BUILT_SOURCES = \
	champlain-marshal.h 		\
	champlain-marshal.c

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list

lib_LTLIBRARIES = libchamplain-0.2.la

libchamplain_0_2_la_SOURCES = \
	$(CHAMPLAIN_MARSHAL_LIST) 	\
	$(BUILT_SOURCES) 		\
	champlain-debug.c 		\
	champlain-view.c 		\
	champlain-layer.c 		\
	champlain-marker.c 		\
	champlain-map.c  		\
	champlain-zoom-level.c 		\
	champlain-tile.c 		\
	sources/oam.c 			\
	sources/osmmapnik.c 		\
	sources/mffrelief.c 		\
	sources/googlemap.c 		\
	sources/googlesat.c 		\
	sources/googleterrain.c 	\
	sources/debugmap.c

noinst_HEADERS = \
	champlain-debug.h  		\
	champlain-view.h 		\
	champlain-marker.c 		\
	champlain-private.h 		\
	champlain-map.h  		\
	champlain-zoom-level.h 		\
	champlain-tile.h 		\
	sources/oam.h 			\
	sources/osmmapnik.h 		\
	sources/mffrelief.h 		\
	sources/googlemap.h 		\
	sources/googlesat.h 		\
	sources/googleterrain.h 	\
	sources/debugmap.h

libchamplain_include_HEADERS = \
	champlain.h 			\
	champlain-view.h 		\
	champlain-defines.h 		\
	champlain-layer.h 		\
	champlain-marker.h

libchamplain_0_2_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la

libchamplain_includedir = $(includedir)/libchamplain-0.2/champlain

libchamplain_0_2_la_LDFLAGS = -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(DEPS_CFLAGS) -DDATADIR=\""$(datadir)"\" -I$(top_srcdir)/tidy -I$(top_srcdir) -DCHAMPLAIN_COMPILATION
AM_LDFLAGS = $(DEPS_LIBS) -export-dynamic -export-symbols-regex ^champlain_.*

svgdir = $(datadir)/champlain
svg_DATA = error.svg

EXTRA_DIST = $(svg_DATA) $(CHAMPLAIN_MARSHAL_LIST)

champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).h

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).c
