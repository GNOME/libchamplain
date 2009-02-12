#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libchamplain, 0.3, pierre-luc@pierlux.com)
AC_CONFIG_SRCDIR([champlain/champlain.h])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

# Checks for libraries.
LIBRARY_VERSION=1:3:0
# API_VERSION is used to generate pc file only
API_VERSION=0.3

AC_SUBST(LIBRARY_VERSION)
AC_SUBST(API_VERSION)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

PKG_CHECK_MODULES(DEPS,
  [   glib-2.0 >= 2.16,
      gobject-2.0 >= 2.10,
      gdk-2.0 >= 2.10,
      clutter-0.8 >= 0.8,
      clutter-cairo-0.8 >= 0.8,
      cairo >= 1.4,
      libsoup-2.4 >= 2.4.1,
      gio-2.0 >= 2.16
  ]
)
# check for gtk-doc
GTK_DOC_CHECK(1.9)

# -----------------------------------------------------------
# Enable debug
# -----------------------------------------------------------

AC_ARG_ENABLE(debug,
  AC_HELP_STRING([--disable-debug],[compile without debug code]),
    enable_debug=$enableval, enable_debug=yes )

if test x$enable_debug = xyes; then
  AC_DEFINE(ENABLE_DEBUG, [], [Enable debug code])
fi

# -----------------------------------------------------------
# Enable debug
# -----------------------------------------------------------

AC_ARG_ENABLE(gtk,
  AC_HELP_STRING([--disable-gtk],[Don't compile Gtk+ embedded view]),
    enable_gtk=$enableval, enable_gtk=yes )

if test x$enable_gtk = xyes; then
  AC_SUBST(GTK_DEPS_CFLAGS)
  AC_SUBST(GTK_DEPS_LIBS)
  PKG_CHECK_MODULES(GTK_DEPS,
    [   gtk+-2.0 >= 2.2
        clutter-gtk-0.8 >= 0.8
    ]
  )

  AC_CONFIG_FILES([champlain-gtk/Makefile
                   docs/reference-gtk/Makefile
                   docs/reference-gtk/version.xml
                   champlain-gtk.pc])

  AC_DEFINE(ENABLE_GTK, [], [Enable Gtk+ view code])
fi

AM_CONDITIONAL(ENABLE_GTK, test "x$enable_gtk" = "xyes")
# -----------------------------------------------------------

AC_CONFIG_FILES([Makefile
                 champlain/Makefile
                 demos/Makefile
                 tidy/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 champlain.pc])
AC_OUTPUT

echo ""
echo " libchamplain $VERSION"
echo ""
echo "         Prefix: ${prefix}"
echo " Compiler flags: ${CPPFLAGS}"
echo "          Debug: ${enable_debug}"
echo "      Gtk+ View: ${enable_gtk}"

