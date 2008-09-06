
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

libchamplain_1_0_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la

EXTRA_DIST = $(CHAMPLAIN_MARSHAL_LIST)
        
champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).h 

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-1.0/champlain
libchamplain_include_HEADERS = champlain.h champlainview.h champlain_defines.h champlainmarker.h

libchamplain_1_0_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

noinst_PROGRAMS = launcher

AM_CPPFLAGS = $(DEPS_CFLAGS) -I../tidy
AM_LDFLAGS = $(DEPS_LIBS) -export-dynamic -export-symbols-regex ^champlain_.*
SOURCES = launcher.c
LDADD = $(DEPS_LIBS) $(top_builddir)/champlain/libchamplain-1.0.la
