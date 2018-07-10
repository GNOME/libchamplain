glib_prefix = glib_dep.get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
clutter_prefix = clutter_dep.get_pkgconfig_variable('prefix')
clutter_docpath = join_paths(clutter_prefix, 'share', 'gtk-doc', 'html')
gtk_prefix = gtk_dep.get_pkgconfig_variable('prefix')
gtk_docpath = join_paths(gtk_prefix, 'share', 'gtk-doc', 'html')
docpath = join_paths(get_option('datadir'), 'gtk-doc', 'html')

gnome.gtkdoc(
  'libchamplain-gtk',
  main_sgml: 'libchamplain-gtk-docs.sgml',
  src_dir: libchamplain_gtk_dir,
  dependencies: libchamplain_gtk_deps,
  scan_args: ['--rebuild-types', '--deprecated-guards="GTK_DISABLE_DEPRECATED"'],
  ignore_headers: 'champlain-gtk-marshal.h',
  fixxref_args: [
    '--html-dir=@0@'.format(docpath),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
    '--extra-dir=@0@'.format(clutter_docpath),
    '--extra-dir=@0@'.format(gtk_docpath),
    '--extra-dir=@0@'.format(join_paths(meson.build_root(), 'docs', 'reference')),
  ],
  install: true
)