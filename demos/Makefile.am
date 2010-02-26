noinst_PROGRAMS = launcher animated-marker polygons url-marker

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = $(DEPS_CFLAGS) $(WARN_CFLAGS)
AM_LDFLAGS = $(DEPS_LIBS)

launcher_SOURCES = launcher.c markers.c
launcher_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.5.la

animated_marker_SOURCES = animated-marker.c
animated_marker_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.5.la

polygons_SOURCES = polygons.c
polygons_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.5.la

url_marker_SOURCES = url-marker.c
url_marker_CPPFLAGS = $(DEPS_CFLAGS) $(SOUP_CFLAGS)
url_marker_LDADD = $(SOUP_LIBS) $(DEPS_LIBS) ../champlain/libchamplain-0.5.la

if ENABLE_GTK
noinst_PROGRAMS += launcher-gtk
launcher_gtk_SOURCES = launcher-gtk.c markers.c
launcher_gtk_CPPFLAGS = $(GTK_DEPS_CFLAGS)
launcher_gtk_LDADD = $(GTK_DEPS_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-0.5.la \
		../champlain/libchamplain-0.5.la

noinst_PROGRAMS += local-rendering
local_rendering_SOURCES = local-rendering.c
local_rendering_CPPFLAGS = $(GTK_DEPS_CFLAGS)
local_rendering_LDADD = $(GTK_DEPS_LIBS) $(DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-0.5.la \
		../champlain/libchamplain-0.5.la
endif

EXTRA_DIST = markers.h

if HAVE_INTROSPECTION
EXTRA_DIST += launcher.js
endif
