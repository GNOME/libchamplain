#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
m4_define([champlain_major_version], [0])
m4_define([champlain_minor_version], [4])
m4_define([champlain_micro_version], [2])
m4_define([champlain_version],
          [champlain_major_version.champlain_minor_version.champlain_micro_version])
m4_define([api_version],
          [champlain_major_version.champlain_minor_version])
# if the API changes, set to 0
AC_PREREQ(2.61)
AC_INIT([libchamplain],
        [champlain_version],
        pierre-luc@pierlux.com)
AC_CONFIG_SRCDIR([champlain/champlain.h])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

# Checks for libraries.
LIBRARY_VERSION=2:0:2
# API_VERSION is used to generate pc files only
API_VERSION=api_version
CHAMPLAIN_MAJOR_VERSION=champlain_major_version
CHAMPLAIN_MINOR_VERSION=champlain_minor_version
CHAMPLAIN_MICRO_VERSION=champlain_micro_version
CHAMPLAIN_VERSION=champlain_version
CHAMPLAIN_MAJORMINOR=champlain_api_version
AC_SUBST(API_VERSION)
AC_SUBST(CHAMPLAIN_MAJOR_VERSION)
AC_SUBST(CHAMPLAIN_MINOR_VERSION)
AC_SUBST(CHAMPLAIN_MICRO_VERSION)
AC_SUBST(CHAMPLAIN_VERSION)
AC_SUBST(CHAMPLAIN_MAJORMINOR)

AC_SUBST(LIBRARY_VERSION)
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

PKG_CHECK_MODULES(DEPS,
  [   glib-2.0 >= 2.16,
      gobject-2.0 >= 2.10,
      gdk-2.0 >= 2.10,
      clutter-1.0 >= 1.0
      cairo >= 1.4,
      gio-2.0 >= 2.16,
      sqlite3 >= 3.0
  ]
)
# check for gtk-doc
GTK_DOC_CHECK(1.9)
IDT_COMPILE_WARNINGS

# -----------------------------------------------------------
# Check for libsoup, use libsoup-gnome if available
# -----------------------------------------------------------
PKG_CHECK_MODULES(SOUP, libsoup-gnome-2.4 >= 2.26, [
    have_soup_gnome="yes"
    AC_DEFINE(HAVE_LIBSOUP_GNOME,1,[libsoup gnome integration])
    ], have_soup_gnome="no")

if test "x$have_soup_gnome" = "xno"; then
  PKG_CHECK_MODULES(SOUP, libsoup-2.4 >= 2.4.1,
     have_soup="yes", have_soup="no")
fi

if test "x$have_soup" = "xno" && test "x$have_soup_gnome" = "xno"; then
   AC_MSG_ERROR([Couldn't find libsoup or libsoup-gnome.])
fi

AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)

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
# Enable gtk
# -----------------------------------------------------------

AC_ARG_ENABLE(gtk,
  AC_HELP_STRING([--disable-gtk],[Don't compile Gtk+ embedded view]),
    enable_gtk=$enableval, enable_gtk=yes )

if test x$enable_gtk = xyes; then
  AC_SUBST(GTK_DEPS_CFLAGS)
  AC_SUBST(GTK_DEPS_LIBS)
  PKG_CHECK_MODULES(GTK_DEPS,
    [   gtk+-2.0 >= 2.12
        clutter-gtk-0.10 >= 0.10
    ]
  )

  AC_CONFIG_FILES([champlain-gtk/Makefile
                   docs/reference-gtk/Makefile
                   docs/reference-gtk/version.xml
                   champlain-gtk.pc
                   champlain-gtk-uninstalled.pc])

  AC_DEFINE(ENABLE_GTK, [], [Enable Gtk+ view code])
fi

AM_CONDITIONAL(ENABLE_GTK, test "x$enable_gtk" = "xyes")

# -----------------------------------------------------------
# Enable managed (default to "no")
# -----------------------------------------------------------
AC_ARG_ENABLE(managed,
  AC_HELP_STRING([--enable-managed],[Build champlain-sharp (and champlain-gtk-sharp)]),
    enable_managed=$enableval, enable_managed=no )

if test x$enable_managed = xyes; then
  AC_PATH_PROG(MCS, gmcs)
  AC_SUBST(MCS)

  AC_PATH_PROG(GACUTIL, gacutil)
  AC_SUBST(GACUTIL)

  PKG_CHECK_MODULES(GAPI, gapi-2.0 >= 2.12.0)
  AC_PATH_PROG(GAPI_PARSER, gapi2-parser)
  AC_SUBST(GAPI_PARSER)

  AC_PATH_PROG(GAPI_CODEGEN, gapi2-codegen)
  AC_SUBST(GAPI_CODEGEN)

  AC_PATH_PROG(GAPI_FIXUP, gapi2-fixup)
  AC_SUBST(GAPI_FIXUP)

  PKG_CHECK_MODULES(GTKSHARP, gtk-sharp-2.0 > 2.12.0)
  AC_SUBST(GTKSHARP_LIBS)

  PKG_CHECK_MODULES(CLUTTERSHARP, clutter-sharp >= 0.8.0)
  AC_SUBST(CLUTTER_SHARP_LIBS)

  PKG_CHECK_MODULES(CLUTTER_GTK_SHARP, clutter-gtk-sharp >= 0.8.0)
  AC_SUBST(CLUTTER_GTK_SHARP_LIBS)

  CHAMPLAIN_APIVERSION=0.3
  CHAMPLAIN_GTK_APIVERSION=0.3
  AC_SUBST(CHAMPLAIN_APIVERSION)
  AC_SUBST(CHAMPLAIN_GTK_APIVERSION)
fi

AM_CONDITIONAL(ENABLE_MANAGED, test "x$enable_managed" = "xyes")

# -----------------------------------------------------------
# Enable Python bindings
# -----------------------------------------------------------

AC_ARG_ENABLE(python,
  AC_HELP_STRING([--enable-python],[Build python bindings]),
    enable_python=$enableval, enable_python=no )

if test x$enable_python = xyes; then
  PKG_CHECK_MODULES(PYTHON_BINDING,
   [
      pygtk-2.0,
      pygobject-2.0,
      glib-2.0,
      gobject-2.0,
      clutter-1.0,
      pyclutter-1.0,
      pyclutter-gtk-0.9,
      gconf-2.0,
      gtk+-2.0
   ], have_python="yes", have_python="no")

  if test "x$have_python" = "xyes" ; then
    AC_CHECK_PROGS([PYGOBJECTCODEGEN], [pygobject-codegen-2.0 pygtk-codegen-2.0])
    AM_CHECK_PYTHON_HEADERS(,have_python="no")
  fi
fi

if test "x$enable_python" = "xyes" -a "x$have_python" != "xyes"; then
   AC_MSG_ERROR([Couldn't find python. Check config.log])
fi

AM_CONDITIONAL(ENABLE_PYTHON, test "x$enable_python" = "xyes")
GOBJECT_INTROSPECTION_CHECK([0.6.3])

# -----------------------------------------------------------
AC_CONFIG_FILES([Makefile
                 champlain/Makefile
                 champlain/champlain-version.h
                 demos/Makefile
                 tidy/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 champlain.pc
                 champlain-uninstalled.pc
                 bindings/perl/Champlain/Makefile
                 bindings/python/Makefile
                 bindings/python/champlain/Makefile
                 bindings/python/champlain-gtk/Makefile
                 bindings/python/demos/Makefile
                 bindings/managed/Makefile
                 bindings/managed/champlain/Makefile
                 bindings/managed/champlain/AssemblyInfo.cs
                 bindings/managed/champlain/champlain-sharp.pc
                 bindings/managed/champlain/champlain-sharp.dll.config
                 bindings/managed/champlain-gtk/Makefile
                 bindings/managed/champlain-gtk/AssemblyInfo.cs
                 bindings/managed/champlain-gtk/champlain-gtk-sharp.pc
                 bindings/managed/champlain-gtk/champlain-gtk-sharp.dll.config
                 bindings/managed/samples/Makefile
                 bindings/Makefile])
AC_OUTPUT

echo ""
echo " libchamplain $VERSION"
echo ""
echo "          Prefix: ${prefix}"
echo "  Compiler flags: ${CPPFLAGS}"
echo "   Documentation: ${enable_gtk_doc}"
echo "           Debug: ${enable_debug}"
echo "   libsoup-gnome: ${have_soup_gnome}"
echo "       Gtk+ View: ${enable_gtk}"
echo ""
echo "Bindings:"
echo "       champlain-sharp: ${enable_managed}"
echo "       Python bindings: ${enable_python}"
echo " gobject-introspection: ${enable_introspection}"
echo ""

