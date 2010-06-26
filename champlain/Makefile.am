
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
	champlain-memphis-renderer.h
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
	champlain-features.h	\
	champlain-renderer.h	\
	champlain-image-renderer.h	\
	champlain-file-tile-source.h	\
	champlain-network-bbox-tile-source.h	\
	champlain-bounding-box.h

if ENABLE_MEMPHIS
memphis_sources = \
	champlain-memphis-renderer.c
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
	champlain-polygon.c		\
	champlain-renderer.c	\
	champlain-image-renderer.c	\
	champlain-file-tile-source.c	\
	champlain-network-bbox-tile-source.c	\
	champlain-bounding-box.c

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
	champlain-features.h	\
	champlain-renderer.h	\
	champlain-image-renderer.h	\
	champlain-file-tile-source.h	\
	champlain-network-bbox-tile-source.h	\
	champlain-bounding-box.h

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

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS =
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
introspection_sources = \
	$(libchamplain_@CHAMPLAIN_API_VERSION@_la_SOURCES) \
	$(libchamplain_include_HEADERS)

if ENABLE_MEMPHIS
memphis_gir_include = Memphis-0.2
endif

Champlain-@CHAMPLAIN_API_VERSION@.gir: libchamplain-@CHAMPLAIN_API_VERSION@.la
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_NAMESPACE = Champlain
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_SCANNERFLAGS = --strip-prefix=Champlain
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_VERSION = @CHAMPLAIN_API_VERSION@
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_LIBS = libchamplain-@CHAMPLAIN_API_VERSION@.la
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_PACKAGES = gobject-2.0 gtk+-2.0
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_FILES = $(addprefix $(srcdir)/,$(introspection_sources))
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_INCLUDES = Clutter-1.0 Gtk-2.0 $(memphis_gir_include)
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_CFLAGS = \
	$(DEPS_CFLAGS) $(SOUP_CFLAGS) $(MEMPHIS_CFLAGS) \
	-I$(top_srcdir)/tidy -I$(top_srcdir) \
	-DCHAMPLAIN_COMPILATION

INTROSPECTION_GIRS += Champlain-@CHAMPLAIN_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif


