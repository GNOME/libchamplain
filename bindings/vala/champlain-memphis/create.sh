export PKG_CONFIG_PATH=$(readlink -f ../../../)

vala-gen-introspect champlain-memphis-0.6 champlain-memphis-0.6
vapigen --library champlain-memphis-0.6 --pkg champlain-0.6 --vapidir=../champlain champlain-memphis-0.6/champlain-memphis-0.6.gi --metadata champlain-memphis-0.6/champlain-memphis-0.6.metadata
