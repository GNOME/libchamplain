noinst_PROGRAMS = launcher

launcher_SOURCES = \
	launcher.vala \
	markers.vala

INCLUDES = \
	-I$(top_srcdir) \
	$(DEPS_CFLAGS)

VALAFLAGS = \
	--vapidir=$(srcdir)/../champlain \
	--pkg clutter-1.0 --pkg champlain-@CHAMPLAIN_API_VERSION@

launcher_LDADD = \
	$(DEPS_LIBS) \
	$(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

