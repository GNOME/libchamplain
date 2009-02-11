AC_INIT(README)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(champlain-sharp, 0.3)

AM_MAINTAINER_MODE

AC_PATH_PROG(MONO, mono, no)
if test "x$MONO" = "xno"; then
	AC_MSG_ERROR([Cannot find the "mono" runtime in your PATH])
fi

AC_PATH_PROG(MCS, gmcs, no)
if test "x$MCS" = "xno"; then
	AC_MSG_ERROR([Cannot find the "gmcs" compiler in your PATH])
fi

AC_PATH_PROG(GACUTIL, gacutil, no)
if test "x$GACUTIL" = "xno"; then
	AC_MSG_ERROR([Cannot find the "gacutil" program in your PATH])
fi

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" = "xno"; then
	AC_MSG_ERROR([Cannot find the "pkg-config" program in your PATH])
fi

AC_ARG_ENABLE(docs,
              AC_HELP_STRING([--disable-docs],
                             [Do not build Monodoc documentation]),
              with_docs=no, with_docs=yes)

if test "x$with_docs" = "xyes"; then
	AC_PATH_PROG(MONODOCER, monodocer, no)
	if test "x$MONODOCER" = "xno"; then
		AC_MSG_ERROR([You need to install monodoc])
	fi

	AC_PATH_PROG(MDASSEMBLER, mdassembler, no)
	if test "x$MDASSEMBLER" = "xno"; then
		AC_MSG_ERROR([You need to install mdassembler])
	fi

	DOCDIR=`$PKG_CONFIG monodoc --variable=sourcesdir`
	AC_SUBST(DOCDIR)

	AM_CONDITIONAL(BUILD_DOCS, true)
else
	AC_MSG_NOTICE([not building API documentation])
	AM_CONDITIONAL(BUILD_DOCS, false)
fi

PKG_CHECK_MODULES(GAPI, gapi-2.0 >= 2.12.1)
AC_PATH_PROG(GAPI_PARSER, gapi2-parser, no)
AC_SUBST(GAPI_PARSER)
AC_PATH_PROG(GAPI_CODEGEN, gapi2-codegen, no)
AC_SUBST(GAPI_CODEGEN)
AC_PATH_PROG(GAPI_FIXUP, gapi2-fixup, no)
AC_SUBST(GAPI_FIXUP)

PKG_CHECK_MODULES(GTKSHARP, gtk-sharp-2.0 > 2.3.92)
AC_SUBST(GTKSHARP_LIBS)

PKG_CHECK_MODULES(CLUTTERSHARP, clutter-sharp >= 0.8.0)
AC_SUBST(CLUTTER_SHARP_LIBS)

PKG_CHECK_MODULES(CLUTTER_GTK_SHARP, clutter-gtk-sharp >= 0.8.0)
AC_SUBST(CLUTTER_GTK_SHARP_LIBS)

PKG_CHECK_MODULES(CHAMPLAIN, champlain-0.3 >= 0.3)

CHAMPLAIN_APIVERSION=$(pkg-config --variable apiversion champlain-0.3)

CHAMPLAIN_GTK_REQUIRED_VERSION=0.3
PKG_CHECK_MODULES(CHAMPLAIN_GTK, champlain-gtk-0.3 >= $CHAMPLAIN_GTK_REQUIRED_VERSION, enable_gtk=yes, enable_gtk=no)

AM_CONDITIONAL(ENABLE_GTK, test "x$enable_gtk" = "xyes")

AC_SUBST(CHAMPLAIN_APIVERSION)

AC_OUTPUT([
	Makefile
	champlain-cil/champlain-sharp.pc
	champlain-cil/Makefile
	champlain-cil/AssemblyInfo.cs
	champlain-cil/champlain-sharp.dll.config
	champlain-gtk-cil/champlain-gtk-sharp.pc
	champlain-gtk-cil/Makefile
	champlain-gtk-cil/AssemblyInfo.cs
	champlain-gtk-cil/champlain-gtk-sharp.dll.config
	examples/Makefile
])

