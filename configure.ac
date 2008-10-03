#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libchamplain-gtk, 0.2.2, pierre-luc@pierlux.com)
AC_CONFIG_SRCDIR([champlain-gtk/champlainviewembed.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR(m4)


AM_INIT_AUTOMAKE
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

# Checks for libraries.
LIBRARY_VERSION=0:1:0

AC_SUBST(LIBRARY_VERSION)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

PKG_CHECK_MODULES(DEPS,
  [    gtk+-2.0 >= 2.2,
      clutter-0.8 >= 0.8,
      clutter-gtk-0.8 >= 0.8,
      champlain-1.0 >= 0.2
  ]
)
# check for gtk-doc
GTK_DOC_CHECK(1.9)

AC_CONFIG_FILES([Makefile
                 champlain-gtk/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 champlain-gtk.pc])
AC_OUTPUT

echo ""
echo " libchamplain-gtk $VERSION"
echo ""
echo "         Prefix: ${prefix}"
echo " Compiler flags: ${CPPFLAGS}"
echo ""
