glib_prefix = glib_dep.get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
clutter_prefix = clutter_dep.get_pkgconfig_variable('prefix')
clutter_docpath = join_paths(clutter_prefix, 'share', 'gtk-doc', 'html')
docpath = join_paths(get_option('datadir'), 'gtk-doc', 'html')

ignored_headers = [
  'champlain-debug.h',
  'champlain-enum-types.h',
  'champlain-private.h',
  'champlain.h',
  'champlain-marshal.h',
  'champlain-defines.h',
  'champlain-features.h',
  'champlain-adjustment.h',
  'champlain-kinetic-scroll-view.h',
  'champlain-viewport.h'
]

if get_option('memphis')
 ignored_headers += 'champlain-memphis-renderer.h'
endif

gnome.gtkdoc(
  'libchamplain',
  main_sgml: 'libchamplain-docs.sgml',
  src_dir: libchamplain_dir,
  dependencies: libchamplain_deps,
  scan_args: ['--rebuild-types', '--deprecated-guards="GTK_DISABLE_DEPRECATED"'],
  ignore_headers: ignored_headers,
  fixxref_args: [
    '--html-dir=@0@'.format(docpath),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
    '--extra-dir=@0@'.format(clutter_docpath),
  ],
  install: true
)