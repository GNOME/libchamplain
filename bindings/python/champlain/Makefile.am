PYCLUTTERDEFS=`pkg-config --variable=defsdir pyclutter-0.9`
PYGTKDEFS=`pkg-config --variable=defsdir pygtk-2.0`

AM_CPPFLAGS =						\
	-I.						\
	-I$(top_srcdir)/champlain			\
	-I$(top_srcdir)					\
	-DDATADIR=\""$(datadir)"\"			\
	$(PYTHON_INCLUDES)				\
	$(PYTHON_BINDING_CFLAGS)			\
	$(WARN_CFLAGS)

BUILT_SOURCES = 					\
	pychamplain.c					

pychamplaindir = $(pyexecdir)
pychamplain_LTLIBRARIES = champlain.la

champlain_la_SOURCES =					\
	pychamplainmodule.c				\
	pychamplain.c

champlain_la_LIBADD =					\
	$(PYTHON_BINDING_LIBS)				\
	$(top_builddir)/champlain/libchamplain-0.3.la

champlain_la_LDFLAGS =					\
	-module -avoid-version

pychamplain.c: pychamplain.override pychamplain.defs
	$(PYGOBJECTCODEGEN)				\
		--prefix champlain 			\
		--register $(PYGTKDEFS)/pango-types.defs \
		--register $(PYCLUTTERDEFS)/clutter-types.defs \
		--register $(PYCLUTTERDEFS)/cogl-types.defs \
		--override pychamplain.override 	\
		pychamplain.defs > $@

EXTRA_DIST =			\
	pychamplain.override	\
	pychamplain.defs	\
	pychamplain.h

CLEANFILES = $(BUILT_SOURCES)

