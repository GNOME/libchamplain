#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(champlain, 0.1, pierre-luc@squidy.info)
AC_CONFIG_SRCDIR([src/champlainview.h])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AM_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

# Checks for libraries.

AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

PKG_CHECK_MODULES(DEPS,
  [   glib-2.0 >= 2.2,
      gobject-2.0 >= 2.10,
      gtk+-2.0 >= 2.2,
      clutter-0.8 >= 0.8,
      clutter-gtk-0.8 >= 0.8
  ]
)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tidy/Makefile
                 champlain.pc])
AC_OUTPUT


echo ""
echo " Champlain $VERSION"
echo ""
echo "         Prefix: ${prefix}"
echo " Compiler flags: ${CPPFLAGS}"
echo ""
