gtk_prefix = gtk_dep.get_pkgconfig_variable('prefix')
gtk_docpath = join_paths(gtk_prefix, 'share', 'gtk-doc', 'html', 'gtk3')

libchamplain_gtk_reference_ignored_h = [
  'champlain-gtk-marshal.h',
  'champlain-gtk.h',
]

libchamplain_gtk_reference_source_folders = [
  join_paths(srcdir, 'champlain-gtk'),
  join_paths(builddir, 'champlain-gtk'),
]

version_xml = configuration_data()
version_xml.set('PACKAGE_VERSION', version)

configure_file(
  input: 'version.xml.in',
  output: 'version.xml',
  configuration: version_xml,
  install: false,
)

libchamplain_gtk_reference_scan_args = [
  '--rebuild-types',
  '--deprecated-guards=GTK_DISABLE_DEPRECATED',
  '--ignore-headers=' + ' '.join(libchamplain_gtk_reference_ignored_h),
]

libchamplain_gtk_reference_mkdb_args = [
  '--sgml-mode',
  '--output-format=xml',
  '--name-space=gtk_champlain',
]

libchamplain_gtk_reference_fixxref_args = [
  '--html-dir=@0@'.format(join_paths(gtkdocdir, 'html', package_gtk_string)),
  '--extra-dir=@0@'.format(gtk_docpath),
  '--extra-dir=@0@'.format(join_paths(gtkdocdir, 'html', package_string)),
]

gnome.gtkdoc(
  package_gtk_name,
  main_sgml: 'libchamplain-gtk-docs.sgml',
  gobject_typesfile: files('libchamplain-gtk.types'),
  src_dir: libchamplain_gtk_reference_source_folders,
  dependencies: libchamplain_gtk_dep,
  scan_args: libchamplain_gtk_reference_scan_args,
  mkdb_args: libchamplain_gtk_reference_mkdb_args,
  fixxref_args: libchamplain_gtk_reference_fixxref_args,
  install: true,
  install_dir: join_paths(
    gtkdocdir,
    'html',
    package_gtk_string,
  )
)