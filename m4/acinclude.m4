
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


