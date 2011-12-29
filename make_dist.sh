./autogen.sh --enable-gtk --enable-memphis --enable-gtk-doc --enable-introspection --enable-vala --enable-vala-demos
make -j4
make distcheck

