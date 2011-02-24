BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =


tidy_headers_public = \
	$(srcdir)/tidy-adjustment.h		\
	$(srcdir)/tidy-finger-scroll.h		\
	$(srcdir)/tidy-viewport.h		\
	$(srcdir)/tidy-private.h

tidy_sources = \
	$(srcdir)/tidy-adjustment.c \
	$(srcdir)/tidy-finger-scroll.c \
	$(srcdir)/tidy-viewport.c


# glib-genmarshal rules
glib_marshal_list = tidy-marshal.list
glib_marshal_prefix = _tidy_marshal
include $(top_srcdir)/build/Makefile.am.marshal

# glib-mkenums rules
glib_enum_h = tidy-enum-types.h
glib_enum_c = tidy-enum-types.c
glib_enum_headers = $(tidy_headers_public)
include $(top_srcdir)/build/Makefile.am.enums

tidy_headers_built = 	\
	tidy-enum-types.h	\
	tidy-marshal.h

tidy_sources_built = 	\
	tidy-enum-types.c	\
	tidy-marshal.c


libtidy_1_0_la_SOURCES = \
	$(tidy_headers_public)	\
	$(tidy_sources)

nodist_libtidy_1_0_la_SOURCES = \
	$(tidy_headers_built)	\
	$(tidy_sources_built)

libtidy_1_0_la_LIBADD = $(DEPS_LIBS)

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-DPREFIX=\""$(prefix)"\" 	\
	-DLIBDIR=\""$(libdir)"\" 	\
	-DG_DISABLE_DEPRECATED 		\
	-DG_LOG_DOMAIN=\"Tidy\" 	\
	$(DEPS_CFLAGS) 			\
	$(TIDY_DEBUG_CFLAGS)

noinst_LTLIBRARIES = libtidy-1.0.la
