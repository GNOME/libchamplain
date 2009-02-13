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
# Enable gtk
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

  PKG_CHECK_MODULES(GAPI, gapi-2.0 >= 2.12.1)
  AC_PATH_PROG(GAPI_PARSER, gapi2-parser)
  AC_SUBST(GAPI_PARSER)

  AC_PATH_PROG(GAPI_CODEGEN, gapi2-codegen)
  AC_SUBST(GAPI_CODEGEN)

  AC_PATH_PROG(GAPI_FIXUP, gapi2-fixup)
  AC_SUBST(GAPI_FIXUP)
  
  PKG_CHECK_MODULES(GTKSHARP, gtk-sharp-2.0 > 2.3.92)
  AC_SUBST(GTKSHARP_LIBS)
  
  PKG_CHECK_MODULES(CLUTTERSHARP, clutter-sharp >= 0.8.0)
  AC_SUBST(CLUTTER_SHARP_LIBS)
  
  PKG_CHECK_MODULES(CLUTTER_GTK_SHARP, clutter-gtk-sharp >= 0.8.0)
  AC_SUBST(CLUTTER_GTK_SHARP_LIBS)

  CHAMPLAIN_APIVERSION=0.3
  CHAMPLAIN_GTK_APIVERSION=0.2
  AC_SUBST(CHAMPLAIN_APIVERSION)
  AC_SUBST(CHAMPLAIN_GTK_APIVERSION)

AC_CONFIG_FILES([
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
])


  if test x$enable_gtk = xyes; then
    enable_gtk_managed = yes
  fi
fi

AM_CONDITIONAL(ENABLE_MANAGED, test "x$enable_managed" = "xyes")
AM_CONDITIONAL(ENABLE_GTK_MANAGED, test "x$enable_managed" = "xyes" && test "x$enable_gtk" = "xyes")

# -----------------------------------------------------------

AC_CONFIG_FILES([Makefile
                 champlain/Makefile
                 demos/Makefile
                 tidy/Makefile
                 docs/reference/Makefile
                 docs/reference/version.xml
                 champlain.pc
		 bindings/Makefile])
AC_OUTPUT

echo ""
echo " libchamplain $VERSION"
echo ""
echo "          Prefix: ${prefix}"
echo "  Compiler flags: ${CPPFLAGS}"
echo "           Debug: ${enable_debug}"
echo "       Gtk+ View: ${enable_gtk}"
echo ""
echo "Bindings:"
echo " champlain-sharp: ${enable_managed}"
echo ""

