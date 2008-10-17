noinst_PROGRAMS = launcher

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = $(DEPS_CFLAGS) 
AM_LDFLAGS = $(DEPS_LIBS) 

launcher_SOURCES = launcher.c
launcher_LDADD = $(DEPS_LIBS) ../champlain-gtk/libchamplain-gtk-0.2.la
