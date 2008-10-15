
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c 

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list 

lib_LTLIBRARIES = libchamplain-1.0.la

libchamplain_1_0_la_SOURCES = $(CHAMPLAIN_MARSHAL_LIST) \
					$(BUILT_SOURCES) \
					champlainview.c \
					champlainlayer.c \
					champlainmarker.c \
					map.c  \
					zoomlevel.c \
					tile.c \
					sources/oam.c \
					sources/osm_mapnik.c \
					sources/mff_relief.c \
					sources/google_map.c \
					sources/google_sat.c \
					sources/google_terrain.c \
					sources/debugmap.c

noinst_HEADERS = champlainview.h \
					champlainmarker.c \
					champlain_private.h \
					map.h  \
					zoomlevel.h \
					tile.h \
					sources/oam.h \
					sources/osm_mapnik.h \
					sources/mff_relief.h \
					sources/google_map.h \
					sources/google_sat.h \
					sources/google_terrain.h \
					sources/debugmap.h

libchamplain_1_0_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la

champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).h 

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-1.0/champlain
libchamplain_include_HEADERS = champlain.h champlainview.h champlain_defines.h champlainlayer.h champlainmarker.h

libchamplain_1_0_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(DEPS_CFLAGS) -DDATADIR=\""$(datadir)"\" -I$(top_srcdir)/tidy -I$(top_srcdir)
AM_LDFLAGS = $(DEPS_LIBS) -export-dynamic -export-symbols-regex ^champlain_.*

svgdir = $(datadir)/champlain
svg_DATA = error.svg

EXTRA_DIST = $(svg_DATA) $(CHAMPLAIN_MARSHAL_LIST)
