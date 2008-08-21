
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
					map.c  \
					zoomlevel.c \
					tile.c \
					sources/openstreetmap.c \
					sources/debugmap.c 

libchamplain_1_0_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la

EXTRA_DIST = $(CHAMPLAIN_MARSHAL_LIST)
        
champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).h 

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).c

libchamplain_includedir=$(includedir)/libchamplain-1.0/champlain
libchamplain_include_HEADERS = champlain.h

libchamplain_configdir = $(libdir)/libchamplain-1.0/champlain
libchamplain_config_DATA = ../config.h

RELEASE=0:1:0
LIBRARY_VERSION=0:1:0

libchamplain_1_0_la_LDFLAGS= -version-info $(LIBRARY_VERSION) -release $(RELEASE)

noinst_PROGRAMS = launcher

AM_CPPFLAGS = $(DEPS_CFLAGS) -I../tidy
AM_LDFLAGS = $(DEPS_LIBS) -export-dynamic 
SOURCES = launcher.c
LDADD = $(DEPS_LIBS) $(top_builddir)/src/libchamplain-1.0.la
