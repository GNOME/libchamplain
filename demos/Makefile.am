noinst_PROGRAMS = minimal launcher animated-marker polygons url-marker create-destroy-test

SUBDIRS = icons

AM_CPPFLAGS = $(DEPS_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir)

minimal_SOURCES = minimal.c
minimal_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

launcher_SOURCES = launcher.c markers.c
launcher_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

animated_marker_SOURCES = animated-marker.c
animated_marker_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

polygons_SOURCES = polygons.c
polygons_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

url_marker_SOURCES = url-marker.c
url_marker_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

create_destroy_test_SOURCES = create-destroy-test.c
create_destroy_test_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

if ENABLE_GTK
noinst_PROGRAMS += minimal-gtk
minimal_gtk_SOURCES = minimal-gtk.c
minimal_gtk_CPPFLAGS = $(GTK_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir)
minimal_gtk_LDADD = $(GTK_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la \
		../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

noinst_PROGRAMS += launcher-gtk
launcher_gtk_SOURCES = launcher-gtk.c markers.c
launcher_gtk_CPPFLAGS = $(GTK_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir)
launcher_gtk_LDADD = $(GTK_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la \
		../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la

if ENABLE_MEMPHIS
noinst_PROGRAMS += local-rendering
local_rendering_SOURCES = local-rendering.c
local_rendering_CPPFLAGS = $(GTK_CFLAGS) $(MEMPHIS_CFLAGS) $(WARN_CFLAGS) -I$(top_srcdir)
local_rendering_LDADD = $(GTK_LIBS) $(MEMPHIS_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-@CHAMPLAIN_API_VERSION@.la \
		../champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la
endif
endif

if ENABLE_VALA_DEMOS
noinst_PROGRAMS += launcher-vala
launcher_vala_SOURCES = launcher-vala.vala markers-vala.vala
AM_VALAFLAGS = \
  --vapidir=../champlain \
  --pkg atk --pkg clutter-1.0 --pkg champlain-@CHAMPLAIN_API_VERSION@
launcher_vala_LDADD = \
       $(DEPS_LIBS) \
       $(top_builddir)/champlain/libchamplain-@CHAMPLAIN_API_VERSION@.la
endif

EXTRA_DIST = markers.h launcher.js minimal.py launcher-gtk.py polygons.py keyboard-mapping.py default-rules.xml high-contrast.xml las_palmas.osm schaffhausen.osm

