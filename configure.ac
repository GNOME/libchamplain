#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(libchamplain, 0.2.2, pierre-luc@pierlux.com)
AC_CONFIG_SRCDIR([champlain/champlainview.h])
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
LIBRARY_VERSION=1:2:0

AC_SUBST(LIBRARY_VERSION)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

PKG_CHECK_MODULES(DEPS,
  [   glib-2.0 >= 2.16,
      gobject-2.0 >= 2.10,
      gdk-2.0 >= 2.10,
      clutter-0.8 >= 0.8,
      clutter-cairo-0.8 >= 0.8,
      cairo >= 1.6,
      libsoup-2.4 >= 2.4.1,
      gio-2.0 >= 2.16
  ]
)
# check for gtk-doc
GTK_DOC_CHECK(1.9)

AC_CONFIG_FILES([Makefile
                 champlain/Makefile
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
echo ""
