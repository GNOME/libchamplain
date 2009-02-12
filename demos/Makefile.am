noinst_PROGRAMS = launcher launcher-gtk animated-marker

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = $(DEPS_CFLAGS)
AM_LDFLAGS = $(DEPS_LIBS)

launcher_SOURCES = launcher.c
launcher_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.3.la

animated_marker_SOURCES = animated-marker.c
animated_marker_LDADD = $(DEPS_LIBS) ../champlain/libchamplain-0.3.la

launcher_gtk_SOURCES = launcher-gtk.c
launcher_gtk_LDADD = $(DEPS_LIBS) $(CPLAIN_DEPS_LIBS) \
		../champlain-gtk/libchamplain-gtk-0.3.la \
		../champlain/libchamplain-0.3.la
