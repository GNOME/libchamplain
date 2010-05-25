noinst_PROGRAMS = launcher

launcher_SOURCES = \
	launcher.vala \
	markers.vala

INCLUDES = \
	-I$(top_srcdir) \
	$(LAUNCHER_CFLAGS)

VALAFLAGS = \
	--vapidir=$(srcdir)/../champlain \
	$(LAUNCHER_PACKAGES)

launcher_LDADD = \
	$(LAUNCHER_LIBS) \
	$(top_builddir)/champlain/libchamplain-0.6.la
