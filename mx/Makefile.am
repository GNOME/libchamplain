BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =


mx_headers_public = \
	$(srcdir)/mx-draggable.h		\
	$(srcdir)/mx-viewport.h		\
	$(srcdir)/mx-kinetic-scroll-view.h		\
	$(srcdir)/mx-types.h		\
	$(srcdir)/mx-adjustment.h		\
	$(srcdir)/mx-bin.h		\
	$(srcdir)/mx-scrollable.h		\
	$(srcdir)/mx-private.h

mx_sources = \
	$(srcdir)/mx-draggable.c		\
	$(srcdir)/mx-viewport.c		\
	$(srcdir)/mx-kinetic-scroll-view.c		\
	$(srcdir)/mx-adjustment.c		\
	$(srcdir)/mx-bin.c		\
	$(srcdir)/mx-scrollable.c


# glib-genmarshal rules
glib_marshal_list = mx-marshal.list
glib_marshal_prefix = _mx_marshal
include $(top_srcdir)/build/Makefile.am.marshal

# glib-mkenums rules
glib_enum_h = mx-enum-types.h
glib_enum_c = mx-enum-types.c
glib_enum_headers = $(mx_headers_public)
include $(top_srcdir)/build/Makefile.am.enums

mx_headers_built = 	\
	mx-enum-types.h	\
	mx-marshal.h

mx_sources_built = 	\
	mx-enum-types.c	\
	mx-marshal.c


libchamplainmx_1_0_la_SOURCES = \
	$(mx_headers_public)	\
	$(mx_sources)

nodist_libchamplainmx_1_0_la_SOURCES = \
	$(mx_headers_built)	\
	$(mx_sources_built)

libchamplainmx_1_0_la_LIBADD = $(DEPS_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPREFIX=\""$(prefix)"\" 	\
	-DLIBDIR=\""$(libdir)"\" 	\
	-DG_DISABLE_DEPRECATED 		\
	-DG_LOG_DOMAIN=\"Mx\" 	\
	$(DEPS_CFLAGS) 			\
	$(MX_DEBUG_CFLAGS)	\
	$(WARN_CFLAGS)

noinst_LTLIBRARIES = libchamplainmx-1.0.la
