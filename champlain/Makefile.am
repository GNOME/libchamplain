
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c \
	champlain-enum-types.h \
	champlain-enum-types.c \
	stamp-enum-types \
	champlain-version.h

CLEANFILES = $(BUILT_SOURCES)

CHAMPLAIN_MARSHAL = champlain-marshal
CHAMPLAIN_MARSHAL_LIST = champlain-marshal.list

lib_LTLIBRARIES = libchamplain-0.5.la

libchamplain_headers = \
	champlain.h 			\
	champlain-defines.h		\
	champlain-point.h		\
	champlain-view.h		\
	champlain-layer.h 		\
	champlain-selection-layer.h 	\
	champlain-base-marker.h		\
	champlain-marker.h		\
	champlain-map.h			\
	champlain-tile.h		\
	champlain-map-source.h		\
	champlain-map-source-chain.h	\
	champlain-tile-source.h		\
	champlain-tile-cache.h		\
	champlain-network-tile-source.h	\
	champlain-error-tile-source.h	\
	champlain-file-cache.h		\
	champlain-map-source-factory.h	\
	champlain-map-source-desc.h	\
	champlain-polygon.h		\
	champlain-version.h		\
	champlain-memphis-tile-source.h  \
	champlain-map-data-source.h	\
	champlain-local-map-data-source.h \
	champlain-network-map-data-source.h \
	champlain-bounding-box.h

libchamplain_0_5_la_SOURCES = \
	$(CHAMPLAIN_MARSHAL_LIST) 	\
	$(BUILT_SOURCES) 		\
	champlain-debug.c 		\
	champlain-view.c 		\
	champlain-layer.c 		\
	champlain-selection-layer.c 	\
	champlain-base-marker.c 	\
	champlain-marker.c 		\
	champlain-map.c  		\
	champlain-zoom-level.c 		\
	champlain-tile.c		\
	champlain-map-source.c		\
	champlain-map-source-chain.c	\
	champlain-tile-source.c		\
	champlain-tile-cache.c		\
	champlain-network-tile-source.c	\
	champlain-error-tile-source.c	\
	champlain-file-cache.c		\
	champlain-map-source-factory.c	\
	champlain-map-source-desc.c	\
	champlain-point.c		\
	champlain-polygon.c	\
	champlain-memphis-tile-source.c  \
	champlain-map-data-source.c \
	champlain-local-map-data-source.c \
	champlain-network-map-data-source.c \
	champlain-bounding-box.c

noinst_HEADERS = \
	champlain-debug.h  		\
	champlain-view.h 		\
	champlain-base-marker.h 	\
	champlain-marker.h 		\
	champlain-private.h 		\
	champlain-point.h 		\
	champlain-map.h  		\
	champlain-zoom-level.h 		\
	champlain-tile.h 		\
	champlain-enum-types.h		\
	champlain-map-source.h		\
	champlain-map-source-chain.h	\
	champlain-tile-source.h		\
	champlain-tile-cache.h		\
	champlain-network-tile-source.h	\
	champlain-error-tile-source.h	\
	champlain-file-cache.h		\
	champlain-enum-types.h		\
	champlain-map-source-factory.h	\
	champlain-map-source-desc.h	\
	champlain-polygon.h		\
	champlain-version.h

libchamplain_include_HEADERS = \
	champlain.h 			\
	champlain-view.h 		\
	champlain-defines.h 		\
	champlain-point.h 		\
	champlain-enum-types.h		\
	champlain-layer.h 		\
	champlain-selection-layer.h 	\
	champlain-map-source.h		\
	champlain-map-source-chain.h	\
	champlain-tile-source.h		\
	champlain-tile-cache.h		\
	champlain-network-tile-source.h	\
	champlain-error-tile-source.h	\
	champlain-file-cache.h		\
	champlain-map-source-factory.h	\
	champlain-map-source-desc.h	\
	champlain-tile.h		\
	champlain-zoom-level.h		\
	champlain-base-marker.h		\
	champlain-marker.h		\
	champlain-polygon.h		\
	champlain-version.h
	champlain-version.h		\
	champlain-memphis-tile-source.h  \
	champlain-map-data-source.h \
	champlain-local-map-data-source.h \
	champlain-network-map-data-source.h \
	champlain-bounding-box.h

libchamplain_0_5_la_LIBADD = $(DEPS_LIBS) $(SOUP_LIBS) ../tidy/libtidy-1.0.la

libchamplain_includedir = $(includedir)/libchamplain-0.5/champlain

libchamplain_0_5_la_LDFLAGS = -version-info $(LIBRARY_VERSION) \
	-export-symbols-regex		\
	^champlain_.*

AM_CPPFLAGS = 				\
	$(DEPS_CFLAGS)			\
	$(SOUP_CFLAGS)			\
	-DDATADIR=\""$(datadir)"\"	\
	-I$(top_srcdir)/tidy		\
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	$(WARN_CFLAGS)

EXTRA_DIST = $(CHAMPLAIN_MARSHAL_LIST)

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

if HAVE_INTROSPECTION
BUILT_GIRSOURCES =

Champlain-0.5.gir: $(INTROSPECTION_SCANNER) libchamplain-0.5.la
	$(QUIET_GEN)$(INTROSPECTION_SCANNER) -v \
	--namespace Champlain --nsversion=0.5 \
	--strip-prefix=Champlain \
	$(DEPS_CFLAGS) \
	$(SOUP_CFLAGS) \
	-I$(top_srcdir)/tidy		\
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	--add-include-path=$(builddir) \
	--c-include="$(libchamplain_include_HEADERS)" \
	--include=Clutter-1.0 \
	--include=Gtk-2.0 \
	--library=champlain-0.5 \
	--libtool="$(LIBTOOL)" \
	--output $@ \
	--pkg gobject-2.0 \
	--pkg gtk+-2.0 \
	$(addprefix $(srcdir)/,$(libchamplain_0_5_la_SOURCES)) \
	$(addprefix $(srcdir)/,$(libchamplain_include_HEADERS))

BUILT_GIRSOURCES += Champlain-0.5.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

%.typelib: %.gir $(INTROSPECTION_COMPILER)
	$(QUIET_GEN)$(DEBUG) $(INTROSPECTION_COMPILER) --includedir=$(srcdir) --includedir=. $(INTROSPECTION_COMPILER_OPTS) $< -o $(builddir)/$(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif

