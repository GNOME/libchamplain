#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
m4_define([champlain_major_version], [0])
m4_define([champlain_minor_version], [12])
m4_define([champlain_micro_version], [17])
m4_define([champlain_version],
          [champlain_major_version.champlain_minor_version.champlain_micro_version])
m4_define([champlain_minor_api_version], [m4_eval(champlain_minor_version + champlain_minor_version%2)])
m4_define([champlain_api_version],
          [champlain_major_version.champlain_minor_api_version])
AC_PREREQ([2.64])
AC_INIT([libchamplain],
        [champlain_version],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=libchamplain],
        [libchamplain],
        [https://wiki.gnome.org/Projects/libchamplain])
AC_CONFIG_SRCDIR([champlain/champlain.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wno-portability no-dist-gzip dist-xz])

LT_PREREQ([2.2.6])
LT_INIT([dlopen])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Before making a release, the LIBRARY_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
LIBRARY_VERSION=10:0:10
CHAMPLAIN_API_VERSION=champlain_api_version
CHAMPLAIN_API_VERSION_NORM=champlain_major_version[_]champlain_minor_api_version
CHAMPLAIN_MAJOR_VERSION=champlain_major_version
CHAMPLAIN_MINOR_VERSION=champlain_minor_version
CHAMPLAIN_MICRO_VERSION=champlain_micro_version
CHAMPLAIN_VERSION=champlain_version
CHAMPLAIN_MAJORMINOR=champlain_api_version
AC_SUBST(CHAMPLAIN_API_VERSION)
AC_SUBST(CHAMPLAIN_API_VERSION_NORM)
AC_SUBST(CHAMPLAIN_MAJOR_VERSION)
AC_SUBST(CHAMPLAIN_MINOR_VERSION)
AC_SUBST(CHAMPLAIN_MICRO_VERSION)
AC_SUBST(CHAMPLAIN_VERSION)
AC_SUBST(CHAMPLAIN_MAJORMINOR)

AC_SUBST(LIBRARY_VERSION)

# Checks for libraries.

AC_CHECK_LIBM
AC_SUBST(LIBM)

PKG_CHECK_MODULES(DEPS,
  [   glib-2.0 >= 2.16
      gobject-2.0 >= 2.10
      gdk-3.0 >= 2.90
      clutter-1.0 >= 1.12
      cairo >= 1.4
      gio-2.0 >= 2.16
      sqlite3 >= 3.0
      libsoup-2.4 >= 2.42
  ]
)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AM_PATH_GLIB_2_0(2.16.0,,gobject gthread gio)

# check for gtk-doc
GTK_DOC_CHECK(1.9)

GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

GOBJECT_INTROSPECTION_CHECK([0.6.3])

# -----------------------------------------------------------
# Enable debug
# -----------------------------------------------------------

AC_ARG_ENABLE(debug,
  AS_HELP_STRING([--disable-debug],[compile without debug code]),
    enable_debug=$enableval, enable_debug=yes )

if test x$enable_debug = xyes; then
  AC_DEFINE(ENABLE_DEBUG, [], [Enable debug code])
fi

# -----------------------------------------------------------
# Enable gtk
# -----------------------------------------------------------

AC_ARG_ENABLE(gtk,
  AS_HELP_STRING([--disable-gtk],[Do not compile Gtk+ embedded view]),
    enable_gtk=$enableval, enable_gtk=yes )

if test x$enable_gtk = xyes; then
  PKG_CHECK_MODULES(GTK,
    [   gtk+-3.0 >= 2.90
        clutter-gtk-1.0 >= 0.90
    ]
  )
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
fi

AM_CONDITIONAL(ENABLE_GTK, test "x$enable_gtk" = "xyes")

# -----------------------------------------------------------
# Enable memphis
# -----------------------------------------------------------

AC_ARG_ENABLE(memphis,
  AS_HELP_STRING([--disable-memphis],[Disable support of local rendering using libmemphis]),
    enable_memphis=$enableval, enable_memphis=no)

if test "x$enable_memphis" = "xyes"; then
  PKG_CHECK_MODULES(MEMPHIS, [memphis-0.2 >= 0.2.1])
fi

AC_SUBST(MEMPHIS_CFLAGS)
AC_SUBST(MEMPHIS_LIBS)

AM_CONDITIONAL(ENABLE_MEMPHIS, test "x$enable_memphis" = "xyes")

# -----------------------------------------------------------
# Enable vala bindings (default to "no")
# -----------------------------------------------------------

AC_ARG_ENABLE(vala,
  AS_HELP_STRING([--enable-vala], [Build Vala bindings [default=no]]),
    enable_vala=$enableval, enable_vala=no )

if test "x$enable_vala" = "xyes"; then
  if test "x$enable_introspection" = "xno"; then
    AC_MSG_ERROR([Cannot build vala bindings without gobject introspection])    
  fi

  AC_PATH_PROG(VAPIGEN, vapigen, vapigen)
  AC_SUBST(VAPIGEN)
fi

AM_CONDITIONAL(ENABLE_VALA, test "x$enable_vala" = "xyes")

# -----------------------------------------------------------
# Enable vala demos (default to "no")
# -----------------------------------------------------------

AC_ARG_ENABLE(vala-demos,
  AS_HELP_STRING([--enable-vala-demos],[Build vala demos [default=no]]),
    enable_vala_demos=$enableval, enable_vala_demos=no )

if test "x$enable_vala_demos" = "xyes"; then
  AM_PROG_VALAC([0.11.0])

  if test "x$VALAC" = "x" ; then
    AC_MSG_ERROR([Cannot find the "valac" compiler in your PATH])
  fi
fi

AM_CONDITIONAL(ENABLE_VALA_DEMOS, test "x$enable_vala_demos" = "xyes")

# -----------------------------------------------------------
# Generate features header file
# -----------------------------------------------------------

CHAMPLAIN_CONFIG_COMMANDS([champlain/champlain-features.h],
[
  echo '/* Generated by configure.  Do not edit. */'
  echo '#ifndef CHAMPLAIN_FEATURES_H'
  echo '#define CHAMPLAIN_FEATURES_H'
  echo ''
  if test "x$enable_memphis" = "xyes"; then
    echo '#define CHAMPLAIN_HAS_MEMPHIS 1'
    echo ''
  fi
  echo '#endif'
],[
  enable_memphis="$enable_memphis"
])

# ----------------------------------------------------------

AC_CONFIG_FILES([Makefile
                 build/Makefile
                 champlain/Makefile
                 champlain/champlain-version.h
                 demos/Makefile
                 demos/icons/Makefile
                 docs/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 docs/reference-gtk/Makefile
                 docs/reference-gtk/version.xml
                 champlain-$CHAMPLAIN_API_VERSION.pc:champlain.pc.in
                 champlain-$CHAMPLAIN_API_VERSION-uninstalled.pc:champlain-uninstalled.pc.in
                 champlain-gtk-$CHAMPLAIN_API_VERSION.pc:champlain-gtk.pc.in
                 champlain-gtk-$CHAMPLAIN_API_VERSION-uninstalled.pc:champlain-gtk-uninstalled.pc.in
                 champlain-gtk/Makefile])
AC_OUTPUT

echo ""
echo "libchamplain $VERSION"
echo ""
echo "                 Prefix: ${prefix}"
echo "         Compiler flags: ${CFLAGS} ${CPPFLAGS}"
echo "          Documentation: ${enable_gtk_doc}"
echo "                  Debug: ${enable_debug}"
echo "              Gtk+ View: ${enable_gtk}"
echo ""
echo "Extra renderers:"
echo "       Memphis renderer: ${enable_memphis}"
echo ""
echo "Bindings:"
echo "  gobject-introspection: ${enable_introspection}"
echo "   Vala vapi generation: ${enable_vala}"
echo "             Vala demos: ${enable_vala_demos}"
echo ""

