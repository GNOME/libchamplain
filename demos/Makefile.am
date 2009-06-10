noinst_PROGRAMS = launcher animated-marker polygons

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = $(DEPS_CFLAGS) $(WARN_CFLAGS)
AM_LDFLAGS = $(DEPS_LIBS)

launcher_SOURCES = launcher.c markers.c
launcher_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.3.la

animated_marker_SOURCES = animated-marker.c
animated_marker_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.3.la

polygons_SOURCES = polygons.c
polygons_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.3.la

if ENABLE_GTK
noinst_PROGRAMS += launcher-gtk
launcher_gtk_SOURCES = launcher-gtk.c markers.c
launcher_gtk_CPPFLAGS = $(GTK_DEPS_CFLAGS)
launcher_gtk_LDADD = $(GTK_DEPS_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-0.3.la \
		../champlain/libchamplain-0.3.la
endif

EXTRA_DIST= markers.h
