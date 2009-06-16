PYDEFS=`pkg-config --variable=defsdir pygtk-2.0`

AM_CPPFLAGS =                                           \
	-I.                                             \
	-I$(top_srcdir)/champlain-gtk			\
	-I$(top_srcdir)/champlain			\
	-I$(top_srcdir)                                 \
	-DDATADIR=\""$(datadir)"\"              	\
	$(PYTHON_BINDING_CFLAGS)			\
	$(PYTHON_INCLUDES)				\
	$(WARN_CFLAGS)

BUILT_SOURCES = 					\
	pychamplaingtk.c					

pychamplaingtkdir = $(pyexecdir)
pychamplaingtk_LTLIBRARIES = champlaingtk.la

champlaingtk_la_SOURCES =			\
	pychamplaingtkmodule.c			\
	pychamplaingtk.c

champlaingtk_la_LIBADD =					\
	$(PYTHON_BINDING_LIBS)				\
	$(top_builddir)/champlain/libchamplain-0.3.la	\
	$(top_builddir)/champlain-gtk/libchamplain-gtk-0.3.la

champlaingtk_la_LDFLAGS =					\
	-module -avoid-version

pychamplaingtk.c: pychamplaingtk.override pychamplaingtk.defs
	$(PYGOBJECTCODEGEN)				\
		--prefix champlain 			\
		--register $(PYDEFS)/gdk-types.defs 	\
		--register $(PYDEFS)/gtk-types.defs 	\
		--register $(top_srcdir)/bindings/python/champlain/pychamplain.defs \
		--override pychamplaingtk.override 	\
		pychamplaingtk.defs > $@

EXTRA_DIST =			\
	pychamplaingtk.override 			\
	pychamplaingtk.defs

CLEANFILES = $(BUILT_SOURCES)

