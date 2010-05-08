PYCLUTTERDEFS=`pkg-config --variable=defsdir pyclutter-1.0`
PYGTKDEFS=`pkg-config --variable=defsdir pygtk-2.0`
PYMEMPHISDEFS=`pkg-config --variable=defsdir pymemphis-0.2`

AM_CPPFLAGS =						\
	-I.						\
	-I$(top_srcdir)/champlain			\
	-I$(top_srcdir)					\
	-DDATADIR=\""$(datadir)"\"			\
	$(DEPS_CFLAGS) \
	$(MEMPHIS_CFLAGS) \
	$(PYTHON_INCLUDES)				\
	$(PYTHON_BINDING_CFLAGS)			\
	$(PYTHON_BINDING_GTK_CFLAGS)			\
	$(PYTHON_BINDING_MEMPHIS_CFLAGS)		\
	$(WARN_CFLAGS)

BUILT_SOURCES = pychamplain.c

if ENABLE_MEMPHIS
BUILT_SOURCES += pychamplainmemphis.c
endif	

pychamplaindir = $(pyexecdir)
pychamplainmemphisdir = $(pyexecdir)

pychamplain_LTLIBRARIES = champlain.la

if ENABLE_MEMPHIS
pychamplain_LTLIBRARIES += champlainmemphis.la
endif

champlain_la_SOURCES =					\
	pychamplainmodule.c				\
	pychamplain.c
champlainmemphis_la_SOURCES =				\
	pychamplainmemphismodule.c			\
	pychamplainmemphis.c

champlain_la_LIBADD =					\
	$(DEPS_LIBS) \
	$(MEMPHIS_LIBS) \
	$(PYTHON_BINDING_LIBS)				\
	$(PYTHON_BINDING_GTK_LIBS)			\
	$(PYTHON_BINDING_MEMPHIS_LIBS)			\
	$(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la
champlainmemphis_la_LIBADD = $(champlain_la_LIBADD)

champlain_la_LDFLAGS =					\
	-module -avoid-version
champlainmemphis_la_LDFLAGS = $(champlain_la_LDFLAGS)

codegen_register = \
	--register $(PYCLUTTERDEFS)/clutter-types.defs	\
	--register $(PYCLUTTERDEFS)/cogl-types.defs	\
	--register $(PYGTKDEFS)/pango-types.defs

pychamplain.c: pychamplain.override pychamplain.defs
	$(AM_V_GEN)$(PYGOBJECTCODEGEN)		\
		$(codegen_register)	\
		--prefix champlain	\
		--override pychamplain.override		\
		pychamplain.defs > $@

pychamplainmemphis.c: pychamplainmemphis.override pychamplainmemphis.defs
	$(AM_V_GEN)$(PYGOBJECTCODEGEN)		\
		$(codegen_register)	\
		--register $(PYMEMPHISDEFS)/memphis-types.defs	\
		--register pychamplain.defs	\
		--prefix champlainmemphis		\
		--override pychamplainmemphis.override	\
		pychamplainmemphis.defs > $@

EXTRA_DIST =			\
	pychamplain.override	\
	pychamplain-constructors.defs	\
	pychamplain-boxed-types.defs	\
	pychamplain-base.defs	\
	pychamplain.defs	\
	pychamplain.h		\
	pychamplainmemphis.override	\
	pychamplainmemphis-boxed-types.defs	\
	pychamplainmemphis-base.defs	\
	pychamplainmemphis.defs	\
	pychamplainmemphis.h

CLEANFILES = $(BUILT_SOURCES)

