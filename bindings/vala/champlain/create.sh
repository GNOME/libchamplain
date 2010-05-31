export PKG_CONFIG_PATH=$(readlink -f ../../../)

vala-gen-introspect champlain-0.6 champlain-0.6
vapigen --library champlain-0.6 --pkg clutter-1.0 champlain-0.6/champlain-0.6.gi --metadata champlain-0.6/champlain-0.6.metadata
