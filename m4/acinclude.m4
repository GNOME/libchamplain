dnl Turn on the additional warnings last, so -Werror doesn't affect other tests.

AC_DEFUN([IDT_COMPILE_WARNINGS],[
   if test -f $srcdir/autogen.sh; then
	default_compile_warnings="error"
    else
	default_compile_warnings="no"
    fi

    AC_ARG_WITH(compile-warnings,
                AS_HELP_STRING([--with-compile-warnings=@<:@no/yes/error@:>@],
                               [Compiler warnings]),
                [enable_compile_warnings="$withval"],
                [enable_compile_warnings="$default_compile_warnings"])

    warnCFLAGS=
    if test "x$GCC" != xyes; then
	enable_compile_warnings=no
    fi

    warning_flags=
    realsave_CFLAGS="$CFLAGS"

    case "$enable_compile_warnings" in
    no)
	warning_flags=
	;;
    yes)
	warning_flags="-Wall -Wunused -Wmissing-prototypes -Wmissing-declarations"
	;;
    maximum|error)
	warning_flags="-Wall -Wunused -Wmissing-prototypes -Wmissing-declarations -Wchar-subscripts -Wnested-externs -Wpointer-arith"
	CFLAGS="$warning_flags $CFLAGS"
	for option in -Wno-sign-compare -Wno-pointer-sign -Wformat; do
		SAVE_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $option"
		AC_MSG_CHECKING([whether gcc understands $option])
		AC_TRY_COMPILE([], [],
			has_option=yes,
			has_option=no,)
		CFLAGS="$SAVE_CFLAGS"
		AC_MSG_RESULT($has_option)
		if test $has_option = yes; then
		  warning_flags="$warning_flags $option"
		fi
		unset has_option
		unset SAVE_CFLAGS
	done
	unset option
	if test "$enable_compile_warnings" = "error" ; then
	    warning_flags="$warning_flags -Werror"
	fi
	;;
    *)
	AC_MSG_ERROR(Unknown argument '$enable_compile_warnings' to --enable-compile-warnings)
	;;
    esac
    CFLAGS="$realsave_CFLAGS"
    AC_MSG_CHECKING(what warning flags to pass to the C compiler)
    AC_MSG_RESULT($warning_flags)

    WARN_CFLAGS="$warning_flags"
    AC_SUBST(WARN_CFLAGS)
])



dnl CHAMPLAIN_CONFIG_COMMANDS is like AC_CONFIG_COMMANDS, except that:
dnl
dnl     1) It redirects the stdout of the command to the file.
dnl     2) It does not recreate the file if contents did not change.
dnl
dnl (macro copied from CAIRO)

AC_DEFUN([CHAMPLAIN_CONFIG_COMMANDS],
[dnl
        AC_CONFIG_COMMANDS($1,
        [
                _config_file=$1
                _tmp_file=champlainconf.tmp
                AC_MSG_NOTICE([creating $_config_file])
                {
                        $2
                } >> "$_tmp_file" ||
                AC_MSG_ERROR([failed to write to $_tmp_file])

                if cmp -s "$_tmp_file" "$_config_file"; then
                  AC_MSG_NOTICE([$_config_file is unchanged])
                  rm -f "$_tmp_file"
                else
                  mv "$_tmp_file" "$_config_file" ||
                  AC_MSG_ERROR([failed to update $_config_file])
                fi
        ], $3)
])


