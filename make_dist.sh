./autogen.sh --enable-gtk --disable-memphis --enable-gtk-doc --enable-introspection --enable-vala --enable-vala-demos
make -j4
make distcheck

