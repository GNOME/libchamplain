
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c 

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list 

lib_LTLIBRARIES = libchamplain-0.2.la

libchamplain_0_2_la_SOURCES = $(CHAMPLAIN_MARSHAL_LIST) \
					$(BUILT_SOURCES) \
					champlainview.c \
					champlainlayer.c \
					champlainmarker.c \
					map.c  \
					zoomlevel.c \
					tile.c \
					sources/oam.c \
					sources/osmmapnik.c \
					sources/mffrelief.c \
					sources/googlemap.c \
					sources/googlesat.c \
					sources/googleterrain.c \
					sources/debugmap.c

noinst_HEADERS = champlainview.h \
					champlainmarker.c \
					champlainprivate.h \
					map.h  \
					zoomlevel.h \
					tile.h \
					sources/oam.h \
					sources/osmmapnik.h \
					sources/mffrelief.h \
					sources/googlemap.h \
					sources/googlesat.h \
					sources/googleterrain.h \
					sources/debugmap.h

libchamplain_0_2_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la

champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).h 

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-0.2/champlain
libchamplain_include_HEADERS = champlain.h champlainview.h champlaindefines.h champlainlayer.h champlainmarker.h

libchamplain_0_2_la_LDFLAGS= -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(DEPS_CFLAGS) -DDATADIR=\""$(datadir)"\" -I$(top_srcdir)/tidy -I$(top_srcdir) -DCHAMPLAIN_COMPILATION
AM_LDFLAGS = $(DEPS_LIBS) -export-symbols-regex ^champlain_.*

svgdir = $(datadir)/champlain
svg_DATA = error.svg

EXTRA_DIST = $(svg_DATA) $(CHAMPLAIN_MARSHAL_LIST)
