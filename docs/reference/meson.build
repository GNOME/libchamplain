glib_prefix = glib_dep.get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html', 'glib')

gobject_prefix = gobject_dep.get_pkgconfig_variable('prefix')
gobject_docpath = join_paths(gobject_prefix, 'share', 'gtk-doc', 'html', 'gobject')

clutter_prefix = clutter_dep.get_pkgconfig_variable('prefix')
clutter_docpath = join_paths(clutter_prefix, 'share', 'gtk-doc', 'html', 'clutter')

libchamplain_reference_ignored_h = [
  'champlain-adjustment.h',
  'champlain-debug.h',
  'champlain-defines.h',
  'champlain-enum-types.h',
  'champlain-features.h',
  'champlain-kinetic-scroll-view.h',
  'champlain-marshal.h',
  'champlain-private.h',
  'champlain-viewport.h',
  'champlain.h',
]

if build_with_memphis == false
  libchamplain_reference_ignored_h += [
    'champlain-memphis-renderer.h',
  ]
endif

libchamplain_reference_source_folders = [
  join_paths(srcdir, 'champlain'),
  join_paths(builddir, 'champlain'),
]

version_xml = configuration_data()
version_xml.set('PACKAGE_VERSION', version)

configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_xml,
  install: false,
)

libchamplain_reference_scan_args = [
  '--rebuild-types',
  '--deprecated-guards=GTK_DISABLE_DEPRECATED',
  '--ignore-headers=' + ' '.join(libchamplain_reference_ignored_h),
]

libchamplain_reference_mkdb_args = [
  '--sgml-mode',
  '--output-format=xml',
  '--name-space=champlain',
]

libchamplain_reference_fixxref_args = [
  '--html-dir=@0@'.format(join_paths(gtkdocdir, 'html', package_string)),
  '--extra-dir=@0@'.format(glib_docpath),
  '--extra-dir=@0@'.format(gobject_docpath),
  '--extra-dir=@0@'.format(clutter_docpath),
]

gnome.gtkdoc(
  package_name,
  main_sgml: 'libchamplain-docs.sgml',
  gobject_typesfile: files('libchamplain.types'),
  src_dir: libchamplain_reference_source_folders,
  dependencies: libchamplain_dep,
  scan_args: libchamplain_reference_scan_args,
  mkdb_args: libchamplain_reference_mkdb_args,
  fixxref_args: libchamplain_reference_fixxref_args,
  install: true,
  install_dir: join_paths(
    gtkdocdir,
    'html',
    package_string,
  )
)