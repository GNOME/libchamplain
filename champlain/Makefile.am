BUILT_SOURCES = champlain-version.h
CLEANFILES = $(BUILT_SOURCES)
DISTCLEANFILES =
EXTRA_DIST = champlain-version.h.in


if ENABLE_MEMPHIS
memphis_headers = \
	$(srcdir)/champlain-memphis-renderer.h
endif

libchamplain_headers_public = 				\
	$(memphis_headers)				\
	$(srcdir)/champlain-version.h 			\
	$(srcdir)/champlain.h 				\
	$(srcdir)/champlain-defines.h			\
	$(srcdir)/champlain-point.h			\
	$(srcdir)/champlain-view.h			\
	$(srcdir)/champlain-layer.h 			\
	$(srcdir)/champlain-base-marker.h		\
	$(srcdir)/champlain-marker.h			\
	$(srcdir)/champlain-tile.h			\
	$(srcdir)/champlain-map-source.h		\
	$(srcdir)/champlain-map-source-chain.h		\
	$(srcdir)/champlain-tile-source.h		\
	$(srcdir)/champlain-tile-cache.h		\
	$(srcdir)/champlain-memory-cache.h		\
	$(srcdir)/champlain-network-tile-source.h	\
	$(srcdir)/champlain-file-cache.h		\
	$(srcdir)/champlain-map-source-factory.h	\
	$(srcdir)/champlain-map-source-desc.h		\
	$(srcdir)/champlain-polygon.h			\
	$(srcdir)/champlain-renderer.h			\
	$(srcdir)/champlain-image-renderer.h		\
	$(srcdir)/champlain-error-tile-renderer.h	\
	$(srcdir)/champlain-file-tile-source.h		\
	$(srcdir)/champlain-null-tile-source.h		\
	$(srcdir)/champlain-network-bbox-tile-source.h	\
	$(srcdir)/champlain-bounding-box.h

libchamplain_headers_private =	\
	$(srcdir)/champlain-debug.h	\
	$(srcdir)/champlain-private.h


if ENABLE_MEMPHIS
memphis_sources =		\
	$(srcdir)/champlain-memphis-renderer.c
endif

libchamplain_sources =					\
	$(memphis_sources)				\
	$(srcdir)/champlain-debug.c 			\
	$(srcdir)/champlain-view.c 			\
	$(srcdir)/champlain-layer.c 			\
	$(srcdir)/champlain-base-marker.c 		\
	$(srcdir)/champlain-marker.c 			\
	$(srcdir)/champlain-tile.c			\
	$(srcdir)/champlain-map-source.c		\
	$(srcdir)/champlain-map-source-chain.c		\
	$(srcdir)/champlain-tile-source.c		\
	$(srcdir)/champlain-tile-cache.c		\
	$(srcdir)/champlain-memory-cache.c		\
	$(srcdir)/champlain-network-tile-source.c	\
	$(srcdir)/champlain-file-cache.c		\
	$(srcdir)/champlain-map-source-factory.c	\
	$(srcdir)/champlain-map-source-desc.c		\
	$(srcdir)/champlain-point.c			\
	$(srcdir)/champlain-polygon.c			\
	$(srcdir)/champlain-renderer.c			\
	$(srcdir)/champlain-image-renderer.c		\
	$(srcdir)/champlain-error-tile-renderer.c	\
	$(srcdir)/champlain-file-tile-source.c		\
	$(srcdir)/champlain-null-tile-source.c		\
	$(srcdir)/champlain-network-bbox-tile-source.c	\
	$(srcdir)/champlain-bounding-box.c

champlain-features.h: $(top_builddir)/config.status
	$(AM_V_GEN) ( cd $(top_builddir) && ./config.status champlain/$@ )

BUILT_SOURCES += champlain-features.h
CLEANFILES += champlain-features.h

# glib-genmarshal rules
glib_marshal_list = champlain-marshal.list
glib_marshal_prefix = _champlain_marshal
include $(top_srcdir)/build/Makefile.am.marshal

# glib-mkenums rules
glib_enum_h = champlain-enum-types.h
glib_enum_c = champlain-enum-types.c
glib_enum_headers = $(libchamplain_headers_public)
include $(top_srcdir)/build/Makefile.am.enums

libchamplain_headers_built = 	\
	champlain-features.h	\
	champlain-enum-types.h	\
	champlain-marshal.h

libchamplain_sources_built = 	\
	champlain-enum-types.c	\
	champlain-marshal.c


libchamplain_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	$(libchamplain_headers_public)	\
	$(libchamplain_headers_private)	\
	$(libchamplain_sources)

nodist_libchamplain_@CHAMPLAIN_API_VERSION@_la_SOURCES = \
	$(libchamplain_headers_built)	\
	$(libchamplain_sources_built)

libchamplain_@CHAMPLAIN_API_VERSION@_la_LIBADD = $(DEPS_LIBS) $(SOUP_LIBS) $(MEMPHIS_LIBS) $(top_builddir)/mx/libchamplainmx-1.0.la

libchamplain_@CHAMPLAIN_API_VERSION@_la_LDFLAGS = \
	-version-info $(LIBRARY_VERSION)\
	-no-undefined 			\
	-export-symbols-regex ^champlain_.*

AM_CPPFLAGS = 				\
	$(DEPS_CFLAGS)			\
	$(SOUP_CFLAGS)			\
	$(MEMPHIS_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"	\
	-I$(top_srcdir)/mx		\
	-I$(top_srcdir)			\
	-DCHAMPLAIN_COMPILATION 	\
	$(WARN_CFLAGS)


lib_LTLIBRARIES = libchamplain-@CHAMPLAIN_API_VERSION@.la

libchamplaindir = $(includedir)/libchamplain-@CHAMPLAIN_API_VERSION@/champlain

libchamplain_HEADERS = $(libchamplain_headers_public)

nodist_libchamplain_HEADERS = $(libchamplain_headers_built)


if HAVE_INTROSPECTION

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --warn-all
INTROSPECTION_COMPILER_ARGS =
introspection_sources = \
	$(libchamplain_headers_public)	\
	$(libchamplain_sources)
if ENABLE_MEMPHIS
memphis_gir_include = Memphis-0.2
endif

Champlain-@CHAMPLAIN_API_VERSION@.gir: libchamplain-@CHAMPLAIN_API_VERSION@.la
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_NAMESPACE = Champlain
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_VERSION = @CHAMPLAIN_API_VERSION@
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_LIBS = libchamplain-@CHAMPLAIN_API_VERSION@.la
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_PACKAGES = gobject-2.0 gtk+-3.0
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_FILES = $(introspection_sources)
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_INCLUDES = Clutter-1.0 Gtk-3.0 $(memphis_gir_include)
Champlain_@CHAMPLAIN_API_VERSION_NORM@_gir_CFLAGS = \
	$(DEPS_CFLAGS) $(SOUP_CFLAGS) $(MEMPHIS_CFLAGS) \
	-I$(top_srcdir)/mx -I$(top_srcdir) -I$(top_builddir) \
	-DCHAMPLAIN_COMPILATION

INTROSPECTION_GIRS += Champlain-@CHAMPLAIN_API_VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif
