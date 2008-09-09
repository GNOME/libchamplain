NULL =

GLIB_GENMARSHAL = `pkg-config --variable=glib_genmarshal glib-2.0`
GLIB_MKENUMS = `pkg-config --variable=glib_mkenums glib-2.0`

INCLUDES = \
	-I$(top_srcdir) 		\
	-DPREFIX=\""$(prefix)"\" 	\
	-DLIBDIR=\""$(libdir)"\" 	\
	-DG_DISABLE_DEPRECATED 		\
	-DG_LOG_DOMAIN=\"Tidy\" 	\
	$(DEPS_CFLAGS) 			\
	$(TIDY_DEBUG_CFLAGS) 		\
	$(NULL)

LDADD = $(TIDY_LT_LDFLAGS) -export-dynamic -rpath $(libdir)

BUILT_SOURCES = \
	tidy-enum-types.h 	\
	tidy-enum-types.c 	\
	tidy-marshal.h 		\
	tidy-marshal.c

STAMP_FILES = stamp-tidy-marshal.h stamp-tidy-enum-types.h

# please, keep this sorted alphabetically
source_h = \
	$(top_srcdir)/tidy/tidy-adjustment.h		\
	$(top_srcdir)/tidy/tidy-finger-scroll.h		\
	$(top_srcdir)/tidy/tidy-scrollable.h		\
	$(top_srcdir)/tidy/tidy-scroll-view.h		\
	$(top_srcdir)/tidy/tidy-viewport.h		\
	$(top_srcdir)/tidy/tidy-private.h		\
	$(NULL)

source_h_private = \
	tidy-debug.h \
	$(NULL)

# please, keep this sorted alphabetically
source_c = \
	tidy-adjustment.c \
	tidy-finger-scroll.c \
	tidy-scroll-view.c \
	tidy-scrollable.c \
	tidy-viewport.c \
	$(NULL)

tidy-marshal.h: stamp-tidy-marshal.h
	@true
stamp-tidy-marshal.h: Makefile tidy-marshal.list
	$(GLIB_GENMARSHAL) \
		--prefix=_tidy_marshal \
		--header \
	$(srcdir)/tidy-marshal.list > xgen-tmh && \
	(cmp -s xgen-tmh tidy-marshal.h || cp -f xgen-tmh tidy-marshal.h) && \
	rm -f xgen-tmh && \
	echo timestamp > $(@F)

tidy-marshal.c: Makefile tidy-marshal.list
	(echo "#include \"tidy-marshal.h\"" ; \
	 $(GLIB_GENMARSHAL) \
		--prefix=_tidy_marshal \
		--body \
	 $(srcdir)/tidy-marshal.list ) > xgen-tmc && \
	cp -f xgen-tmc tidy-marshal.c && \
	rm -f xgen-tmc

tidy-enum-types.h: stamp-tidy-enum-types.h Makefile
	@true
stamp-tidy-enum-types.h: $(source_h) tidy-enum-types.h.in
	( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template $(srcdir)/tidy-enum-types.h.in \
	  $(source_h) ) >> xgen-teth && \
	(cmp xgen-teth tidy-enum-types.h || cp xgen-teth tidy-enum-types.h) && \
	rm -f xgen-teth && \
	echo timestamp > $(@F)

tidy-enum-types.c: stamp-tidy-enum-types.h tidy-enum-types.c.in
	( cd $(srcdir) && \
	  $(GLIB_MKENUMS) \
	    --template $(srcdir)/tidy-enum-types.c.in \
	  $(source_h) ) >> xgen-tetc && \
	cp xgen-tetc tidy-enum-types.c && \
	rm -f xgen-tetc
	
noinst_libdir = $(top_srcdir)/tidy
noinst_LTLIBRARIES = libtidy-1.0.la

libtidy_1_0_la_LIBADD = $(DEPS_LIBS)
libtidy_1_0_la_SOURCES = \
	$(source_c) \
	$(source_h) \
	$(source_h_priv) \
	$(BUILT_SOURCES) \
	$(NULL)
libtidy_1_0_la_LDFLAGS = $(TIDY_LT_LDFLAGS)

CLEANFILES = $(STAMP_FILES) $(BUILT_SOURCES)

EXTRA_DIST = \
	tidy-enum-types.h.in \
	tidy-enum-types.c.in \
	tidy-marshal.list

