PYDEFS=`pkg-config --variable=defsdir pygtk-2.0`
PYCLUTTERDEFS=`pkg-config --variable=defsdir pyclutter-1.0`

AM_CPPFLAGS =                                           \
	-I.                                             \
	-I$(top_srcdir)/champlain-gtk			\
	-I$(top_srcdir)/champlain			\
	-I$(top_srcdir)                                 \
	-DDATADIR=\""$(datadir)"\"              	\
	$(DEPS_CFLAGS)					\
	$(GTK_CFLAGS)					\
	$(MEMPHIS_CFLAGS)				\
	$(PYTHON_BINDING_CFLAGS)			\
	$(PYTHON_BINDING_GTK_CFLAGS)			\
	$(PYTHON_BINDING_MEMPHIS_CFLAGS)		\
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
	$(PYTHON_BINDING_LIBS)					\
	$(PYTHON_BINDING_GTK_LIBS)				\
	$(PYTHON_BINDING_MEMPHIS_LIBS)				\
	$(DEPS_CFLAGS)						\
	$(GTK_CFLAGS)						\
	$(MEMPHIS_CFLAGS)					\
	$(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la	\
	$(top_builddir)/champlain-gtk/libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la

champlaingtk_la_LDFLAGS =					\
	-module -avoid-version

pychamplaingtk.c: pychamplaingtk.override pychamplaingtk.defs
	$(PYGOBJECTCODEGEN)				\
		--prefix champlain 			\
		--register $(PYDEFS)/gdk-types.defs 	\
		--register $(PYDEFS)/gtk-types.defs 	\
		--register $(PYCLUTTERDEFS)/cluttergtk-types.defs \
		--register $(top_srcdir)/bindings/python/champlain/pychamplain.defs \
		--override pychamplaingtk.override 	\
		pychamplaingtk.defs > $@

EXTRA_DIST =			\
	pychamplaingtk.override 			\
	pychamplaingtk.defs     \
	pychamplaingtk-base.defs

CLEANFILES = $(BUILT_SOURCES)

