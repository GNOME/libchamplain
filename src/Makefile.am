
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c 

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list 

bin_PROGRAMS = champlain

nodist_champlain_SOURCES = \
        $(BUILT_SOURCES)

champlain_SOURCES = $(CHAMPLAIN_MARSHAL_LIST) \
					champlain_widget.c \
					champlain_map.c  \
					champlain_map_zoom_level.c \
					champlain_map_tile.c \
					launcher.c
					

champlain_LDADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la
AM_CPPFLAGS = $(DEPS_CFLAGS) -I../tidy


EXTRA_DIST = $(CHAMPLAIN_MARSHAL_LIST)
        
champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).h 

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=champlain_marshal $< > $(CHAMPLAIN_MARSHAL).c

