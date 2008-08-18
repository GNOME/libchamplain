
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c 

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list 

lib_LTLIBRARIES = libchamplain-1.0.la

bin_PROGRAMS = launcher

nodist_libchamplain_1_0_la_SOURCES = \
        $(BUILT_SOURCES)

launcher_SOURCES = launcher.c
launcher_LDADD = $(DEPS_LIBS) libchamplain-1.0.la

libchamplain_1_0_la_SOURCES = $(CHAMPLAIN_MARSHAL_LIST) \
					champlainview.c \
					map.c  \
					zoomlevel.c \
					tile.c \
					sources/openstreetmap.c \
					sources/debugmap.c 
					

libchamplain_1_0_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la
AM_CPPFLAGS = $(DEPS_CFLAGS) -I../tidy


EXTRA_DIST = $(CHAMPLAIN_MARSHAL_LIST)
        
champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).h 

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).c

