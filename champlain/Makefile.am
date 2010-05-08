
BUILT_SOURCES = \
	champlain-marshal.h \
	champlain-marshal.c \
	champlain-enum-types.h \
	champlain-enum-types.c \
	champlain-version.h \
	champlain-features.h

champlain-features.h: $(top_builddir)/config.status
	$(AM_V_GEN) ( cd $(top_builddir) && ./config.status champlain/$@ )

CLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES = libchamplain-@CHAMPLAIN_API_VERSION@.la

if ENABLE_MEMPHIS
memphis_headers = \
        champlain-memphis.h \
        champlain-memphis-tile-source.h \
        champlain-map-data-source.h     \
        champlain-local-map-data-source.h       \
        champlain-network-map-data-source.h     \
        champlain-bounding-box.h
endif

libchamplain_headers = \
	$(memphis_headers)		\
	champlain.h 			\
	champlain-defines.h		\
	champlain-point.h		\
	champlain-view.h		\
	champlain-layer.h 		\
	champlain-selection-layer.h 	\
	champlain-base-marker.h		\
	champlain-marker.h		\
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
	champlain-features.h

if ENABLE_MEMPHIS
memphis_sources = \
        champlain-memphis-tile-source.c		\
        champlain-map-data-source.c		\
        champlain-local-map-data-source.c	\
        champlain-network-map-data-source.c	\
        champlain-bounding-box.c
endif

libchamplain_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	champlain-marshal.list	 	\
	$(BUILT_SOURCES) 		\
	$(libchamplain_headers)		\
	$(memphis_sources)		\
	champlain-debug.c 		\
	champlain-view.c 		\
	champlain-layer.c 		\
	champlain-selection-layer.c 	\
	champlain-base-marker.c 	\
	champlain-marker.c 		\
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
	champlain-polygon.c

noinst_HEADERS = \
	champlain-debug.h  		\
	champlain-private.h

libchamplain_include_HEADERS = \
	$(memphis_headers)		\
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
	champlain-base-marker.h		\
	champlain-marker.h		\
	champlain-polygon.h		\
	champlain-version.h		\
	champlain-features.h

libchamplain_@CHAMPLAIN_API_VERSION@_la_LIBADD = $(DEPS_LIBS) $(SOUP_LIBS) $(MEMPHIS_LIBS) ../tidy/libtidy-1.0.la

libchamplain_includedir = $(includedir)/libchamplain-@CHAMPLAIN_API_VERSION@/champlain

libchamplain_@CHAMPLAIN_API_VERSION@_la_LDFLAGS = -version-info $(LIBRARY_VERSION) \
	-no-undefined 			\
	-export-symbols-regex		\
	^champlain_.*

AM_CPPFLAGS = 				\
	$(DEPS_CFLAGS)			\
	$(SOUP_CFLAGS)			\
	$(MEMPHIS_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"	\
	-I$(top_srcdir)/tidy		\
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	$(WARN_CFLAGS)

EXTRA_DIST = champlain-marshal.list \
	champlain-enum-types.h.in \
	champlain-enum-types.c.in

champlain-marshal.h: champlain-marshal.list
	$(AM_V_GEN) ( $(GLIB_GENMARSHAL) --header --prefix=cpl_marshal champlain-marshal.list > champlain-marshal.tmp \
	&& mv champlain-marshal.tmp champlain-marshal.h ) \
	|| ( rm -f champlain-marshal.tmp && exit 1 )

champlain-marshal.c: champlain-marshal.h
	$(AM_V_GEN) ( (echo '#include "champlain-marshal.h"'; $(GLIB_GENMARSHAL) --prefix=cpl_marshal $(srcdir)/champlain-marshal.list --body) > champlain-marshal.tmp \
	&& mv champlain-marshal.tmp champlain-marshal.c ) \
	|| ( rm -f champlain-marshal.tmp && exit 1 )

champlain-enum-types.h: $(libchamplain_headers)
	$(AM_V_GEN) ( cd $(srcdir) && $(GLIB_MKENUMS) --template champlain-enum-types.h.in \
		$(libchamplain_headers) ) > champlain-enum-types.h.tmp \
	&& mv champlain-enum-types.h.tmp champlain-enum-types.h \
	|| rm -f champlain-enum-type.h.tmp

champlain-enum-types.c: $(libchamplain_headers) champlain-enum-types.h
	$(AM_V_GEN) ( cd $(srcdir) && $(GLIB_MKENUMS) --template champlain-enum-types.c.in \
		$(libchamplain_headers) ) > champlain-enum-types.c.tmp \
	&& mv champlain-enum-types.c.tmp champlain-enum-types.c \
	|| rm -f champlain-enum-type.c.tmp

if HAVE_INTROSPECTION

if ENABLE_MEMPHIS
memphis_gir_include = --include=Memphis-0.2
endif

BUILT_GIRSOURCES =

Champlain-@CHAMPLAIN_API_VERSION@.gir: $(INTROSPECTION_SCANNER) libchamplain-@CHAMPLAIN_API_VERSION@.la
	$(AM_V_GEN)$(INTROSPECTION_SCANNER) -v \
	--namespace Champlain --nsversion=@CHAMPLAIN_API_VERSION@ \
	--strip-prefix=Champlain \
	$(DEPS_CFLAGS) \
	$(SOUP_CFLAGS) \
	$(MEMPHIS_CFLAGS) \
	-I$(top_srcdir)/tidy		\
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	--add-include-path=$(builddir) \
	--c-include="$(libchamplain_include_HEADERS)" \
	--include=Clutter-1.0 \
	--include=Gtk-2.0 \
	$(memphis_gir_include) \
	--library=champlain-@CHAMPLAIN_API_VERSION@ \
	--libtool="$(LIBTOOL)" \
	--output $@ \
	--pkg gobject-2.0 \
	--pkg gtk+-2.0 \
	$(addprefix $(srcdir)/,$(libchamplain_@CHAMPLAIN_API_VERSION@_la_SOURCES)) \
	$(addprefix $(srcdir)/,$(libchamplain_include_HEADERS))

BUILT_GIRSOURCES += Champlain-@CHAMPLAIN_API_VERSION@.gir

# INTROSPECTION_GIRDIR/INTROSPECTION_TYPELIBDIR aren't the right place to
# install anything - we need to install inside our prefix.
girdir = $(datadir)/gir-1.0
gir_DATA = $(BUILT_GIRSOURCES)

typelibsdir = $(libdir)/girepository-1.0/
typelibs_DATA = $(BUILT_GIRSOURCES:.gir=.typelib)

Champlain-@CHAMPLAIN_API_VERSION@.typelib: Champlain-@CHAMPLAIN_API_VERSION@.gir $(INTROSPECTION_COMPILER)
	$(QUIET_GEN)$(DEBUG) $(INTROSPECTION_COMPILER) --includedir=$(srcdir) --includedir=. $(INTROSPECTION_COMPILER_OPTS) $< -o $(builddir)/$(@F)

CLEANFILES += $(BUILT_GIRSOURCES) $(typelibs_DATA)
endif

