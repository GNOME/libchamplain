#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
m4_define([champlain_major_version], [0])
m4_define([champlain_minor_version], [7])
m4_define([champlain_micro_version], [0])
m4_define([champlain_version],
          [champlain_major_version.champlain_minor_version.champlain_micro_version])
m4_define([champlain_api_version],
          [champlain_major_version.champlain_minor_version])
AC_PREREQ(2.61)
AC_INIT([libchamplain],
        [champlain_version],
        [http://bugzilla.gnome.org/simple-bug-guide.cgi?product=libchamplain])
AC_CONFIG_SRCDIR([champlain/champlain.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wno-portability])
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

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
LIBRARY_VERSION=0:0:0
CHAMPLAIN_API_VERSION=champlain_api_version
CHAMPLAIN_API_VERSION_NORM=champlain_major_version[_]champlain_minor_version
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

PKG_CHECK_MODULES(DEPS,
  [   glib-2.0 >= 2.16
      gobject-2.0 >= 2.10
      gdk-2.0 >= 2.10
      clutter-1.0 >= 1.0
      cairo >= 1.4
      gio-2.0 >= 2.16
      sqlite3 >= 3.0
  ]
)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AM_PATH_GLIB_2_0(2.16.0,,gobject gthread gio)

# check for gtk-doc
GTK_DOC_CHECK(1.9)
IDT_COMPILE_WARNINGS

# -----------------------------------------------------------
# Check for libsoup, use libsoup-gnome if available
# -----------------------------------------------------------
PKG_CHECK_MODULES(SOUP_GNOME, libsoup-gnome-2.4 >= 2.26, [
    have_soup_gnome="yes"
    SOUP_CFLAGS=$SOUP_GNOME_CFLAGS
    SOUP_LIBS=$SOUP_GNOME_LIBS
    SOUP_DEP="libsoup-gnome-2.4"
    AC_DEFINE(HAVE_LIBSOUP_GNOME,1,[libsoup gnome integration])
    ], have_soup_gnome="no")


if test "x$have_soup_gnome" = "xno"; then
  PKG_CHECK_MODULES(SOUP, libsoup-2.4 >= 2.4.1,
     have_soup="yes", have_soup="no")
  SOUP_DEP="libsoup-2.4"
fi

if test "x$have_soup" = "xno" && test "x$have_soup_gnome" = "xno"; then
   AC_MSG_ERROR([Could not find libsoup or libsoup-gnome.])
fi

AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)
AC_SUBST(SOUP_DEP)
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
# Enable Maemo optimizations
# -----------------------------------------------------------

AC_ARG_ENABLE(maemo,
  AS_HELP_STRING([--enable-maemo],[compile with maemo optimizations]),
    [enable_maemo=yes])

# -----------------------------------------------------------
# Enable gtk
# -----------------------------------------------------------

AC_ARG_ENABLE(gtk,
  AS_HELP_STRING([--disable-gtk],[Do not compile Gtk+ embedded view]),
    enable_gtk=$enableval, enable_gtk=yes )

if test x$enable_gtk = xyes; then
  PKG_CHECK_MODULES(GTK,
    [   gtk+-2.0 >= 2.12
        clutter-gtk-0.10 >= 0.10
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
    enable_memphis=$enableval, enable_memphis="auto")

if test "x$enable_memphis" = "xauto"; then
  PKG_CHECK_MODULES(MEMPHIS, [memphis-0.2 >= 0.2.1], enable_memphis="yes", enable_memphis="no")
elif test "x$enable_memphis" = "xyes"; then
  PKG_CHECK_MODULES(MEMPHIS, [memphis-0.2 >= 0.2.1])
fi

AC_SUBST(MEMPHIS_CFLAGS)
AC_SUBST(MEMPHIS_LIBS)

AM_CONDITIONAL(ENABLE_MEMPHIS, test "x$enable_memphis" = "xyes")

# -----------------------------------------------------------
# Enable Python bindings
# -----------------------------------------------------------

AC_ARG_ENABLE(python,
  AS_HELP_STRING([--enable-python],[Build python bindings]),
    enable_python=$enableval, enable_python=no )

if test x$enable_python = xyes; then
  PKG_CHECK_MODULES(PYTHON_BINDING,
   [
      pygtk-2.0 >= 2.12
      pygobject-2.0 >= 2.10
      pyclutter-1.0 >= 1.0
   ]
  )

  if test "x$enable_memphis" = "xyes"; then
    PKG_CHECK_MODULES(PYTHON_BINDING_MEMPHIS, [pymemphis-0.2 >= 0.2])
  fi

  if test "x$enable_gtk" = "xyes"; then
    PKG_CHECK_MODULES(PYTHON_BINDING_GTK, [pyclutter-gtk-0.10 >= 0.10])
  fi

  AC_CHECK_PROGS([PYGOBJECTCODEGEN], [pygobject-codegen-2.0 pygtk-codegen-2.0], [no])

  if test "PYGOBJECTCODEGEN" = "xno"; then
     AC_MSG_ERROR([Could not find pygobject-codegen-2.0 or pygtk-codegen-2.0])
  fi

  AM_CHECK_PYTHON_HEADERS(,[AC_MSG_ERROR(Could not find python headers)])
fi

AM_CONDITIONAL(ENABLE_PYTHON, test "x$enable_python" = "xyes")
GOBJECT_INTROSPECTION_CHECK([0.6.3])

# -----------------------------------------------------------
# Enable vala (default to "no")
# -----------------------------------------------------------

AC_PATH_PROG(VAPIGEN, vapigen, vapigen)
AC_SUBST(VAPIGEN)

AC_ARG_ENABLE(vala,
  AC_HELP_STRING([--enable-vala],[Include vala champlain (and champlain-gtk)]),
    enable_vala=$enableval, enable_vala=no )

if test x$enable_vala = xyes; then
  AM_PROG_VALAC([0.8.0])

  if test "x$VALAC" = "x" ; then
    AC_MSG_ERROR([Cannot find the "valac" compiler in your PATH])
  fi

  LAUNCHER_PACKAGES="--pkg clutter-1.0 --pkg champlain-0.6"
  PKG_CHECK_MODULES(LAUNCHER,
  [
    glib-2.0,
    clutter-1.0
  ])
  AC_SUBST(LAUNCHER_CFLAGS)
  AC_SUBST(LAUNCHER_LIBS)
  AC_SUBST(LAUNCHER_PACKAGES)
fi

AM_CONDITIONAL(ENABLE_VALA, test "x$enable_vala" = "xyes")

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
  if test "x$enable_maemo" = "xyes"; then
    echo '#define CHAMPLAIN_HAS_MAEMO 1'
    echo ''
  fi
  echo '#endif'
],[
  enable_memphis="$enable_memphis"
  enable_maemo="$enable_maemo"
])

# ----------------------------------------------------------

AC_CONFIG_FILES([Makefile
                 champlain/Makefile
                 champlain/champlain-version.h
                 demos/Makefile
                 tidy/Makefile
                 docs/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 docs/reference-gtk/Makefile
                 docs/reference-gtk/version.xml
                 champlain.pc
                 champlain-uninstalled.pc
                 champlain-gtk.pc
                 champlain-gtk-uninstalled.pc
                 champlain-gtk/Makefile
                 bindings/Makefile
                 bindings/python/Makefile
                 bindings/python/champlain/Makefile
                 bindings/python/champlain-gtk/Makefile
                 bindings/python/demos/Makefile
                 bindings/vala/Makefile
                 bindings/vala/champlain/Makefile
                 bindings/vala/champlain-memphis/Makefile
                 bindings/vala/champlain-gtk/Makefile
                 bindings/vala/demos/Makefile
                 champlain-memphis.pc
                 champlain-memphis-uninstalled.pc])
AC_CONFIG_FILES([bindings/python/update-binding.sh],
                [chmod +x bindings/python/update-binding.sh])
AC_OUTPUT

echo ""
echo " libchamplain $VERSION"
echo ""
echo "          Prefix: ${prefix}"
echo "  Compiler flags: ${CFLAGS} ${CPPFLAGS}"
echo "   Documentation: ${enable_gtk_doc}"
echo "           Debug: ${enable_debug}"
echo "   libsoup-gnome: ${have_soup_gnome}"
echo "       Gtk+ View: ${enable_gtk}"
echo ""
echo "Extra tile sources:"
echo "  Memphis source: ${enable_memphis}"
echo ""
echo "Bindings:"
echo "       Python bindings: ${enable_python}"
echo "       Vala bindings: ${enable_vala}"
echo " gobject-introspection: ${enable_introspection}"
echo ""

