
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c \
	champlain-enum-types.h \
	champlain-enum-types.c \
	stamp-enum-types

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list

lib_LTLIBRARIES = libchamplain-0.3.la

libchamplain_headers = \
	champlain.h 			\
	champlain-defines.h		\
	champlain-view.h		\
	champlain-layer.h 		\
	champlain-marker.h		\
	champlain-map.h			\
	champlain-zoom-level.h		\
	champlain-tile.h


libchamplain_0_3_la_SOURCES = \
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
	sources/debugmap.h		\
	champlain-enum-types.h

libchamplain_include_HEADERS = \
	champlain.h 			\
	champlain-view.h 		\
	champlain-defines.h 		\
	champlain-layer.h 		\
	champlain-marker.h

libchamplain_0_3_la_LIBADD = $(DEPS_LIBS) ../tidy/libtidy-1.0.la

libchamplain_includedir = $(includedir)/libchamplain-0.3/champlain

libchamplain_0_3_la_LDFLAGS = -version-info $(LIBRARY_VERSION)

AM_CPPFLAGS = $(DEPS_CFLAGS) -DDATADIR=\""$(datadir)"\" -I$(top_srcdir)/tidy -I$(top_srcdir) -DCHAMPLAIN_COMPILATION
AM_LDFLAGS = $(DEPS_LIBS) -export-dynamic -export-symbols-regex ^champlain_.*

svgdir = $(datadir)/champlain
svg_DATA = error.svg

EXTRA_DIST = $(svg_DATA) $(CHAMPLAIN_MARSHAL_LIST)

champlain-marshal.h: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --header --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).h

champlain-marshal.c: $(CHAMPLAIN_MARSHAL_LIST)
	@GLIB_GENMARSHAL@ --body --prefix=cpl_marshal $< > $(CHAMPLAIN_MARSHAL).c

champlain-enum-types.h: stamp-enum-types
	@true

stamp-enum-types: $(libchamplain_headers) Makefile
	(cd $(srcdir) \
	&& glib-mkenums \
			--fhead "#ifndef __CHAMPLAIN_ENUM_TYPES_H__\n" \
			--fhead "#define __CHAMPLAIN_ENUM_TYPES_H__ 1\n\n" \
			--fhead "#include <glib-object.h>\n\n" \
			--fhead "G_BEGIN_DECLS\n\n" \
			--ftail "G_END_DECLS\n\n" \
			--ftail "#endif /* __CHAMPLAIN_ENUM_TYPES_H__ */\n" \
			--fprod "#include <champlain/@filename@>\n" \
			--eprod "#define CHAMPLAIN_TYPE_@ENUMSHORT@ @enum_name@_get_type()\n" \
			--eprod "GType @enum_name@_get_type (void);\n" \
		$(libchamplain_headers) ) > xgen-gth \
	&& (cmp -s xgen-gth champlain-enum-type.h || cp xgen-gth champlain-enum-types.h) \
	&& rm -f xgen-gth \
	&& echo timestamp > $(@F)

champlain-enum-types.c: $(libchamplain_headers) Makefile
	(cd $(srcdir) \
	&& glib-mkenums \
			--fhead "#include <config.h>\n" \
			--fhead "#include <glib-object.h>\n" \
			--fhead "#include \"champlain-enum-types.h\"\n\n" \
			--fprod "\n/* enumerations from \"@filename@\" */" \
			--vhead "static const G@Type@Value _@enum_name@_values[] = {" \
			--vprod "  { @VALUENAME@, \"@VALUENAME@\", \"@valuenick@\" }," \
			--vtail "  { 0, NULL, NULL }\n};\n\n" \
			--vtail "GType\n@enum_name@_get_type (void)\n{\n" \
			--vtail "  static GType type = 0;\n\n" \
			--vtail "  if (!type)\n" \
			--vtail "    type = g_@type@_register_static (\"@EnumName@\", _@enum_name@_values);\n\n" \
			--vtail "  return type;\n}\n\n" \
		$(libchamplain_headers) ) > xgen-gtc \
	&& cp xgen-gtc $(@F) \
	&& rm -f xgen-gtc
