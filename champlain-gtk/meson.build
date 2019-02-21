libchamplain_gtk_name = package_gtk_string.strip('lib')

libchamplain_gtk_public_h = [
  'champlain-gtk.h',
  'gtk-champlain-embed.h',
]

libchamplain_gtk_sources = [
  'gtk-champlain-embed.c',
]

libchamplain_gtk_requires = [
  gtk_dep,
  clutter_gtk_dep,
  libchamplain_sha,
]

libchamplain_gtk_deps = [
  glib_dep,
  gobject_dep,
  gtk_dep,
  clutter_gtk_dep,
  libchamplain_dep,
]

libchamplain_gtk_srcdir = include_directories('.')

libchamplain_gtk_c_args = [
  '-DHAVE_CONFIG_H',
  '-DCHAMPLAIN_GTK_COMPILATION',
  '-DG_LOG_DOMAIN="libchamplain-gtk"',
]

libchamplain_gtk_link_args = [
]

libchamplain_gtk_includedir = join_paths(
  includedir,
  package_gtk_string,
  'champlain-gtk',
)

install_headers(
  libchamplain_gtk_public_h,
  install_dir: libchamplain_gtk_includedir,
)

libchamplain_gtk_sha = library(
  libchamplain_gtk_name,
  libchamplain_gtk_sources,
  version: lib_version,
  include_directories: rootdir,
  dependencies: libchamplain_gtk_deps,
  c_args: libchamplain_gtk_c_args,
  link_args: libchamplain_gtk_link_args,
  install: true,
  install_dir: libdir,
)

libchamplain_gtk_dep_sources = [
]

if generate_gir
  libchamplain_gtk_gir_includes = [
    'GObject-2.0',
    'Clutter-1.0',
    'Gtk-3.0',
    libchamplain_gir.get(0),
  ]

  libchamplain_gtk_gir = gnome.generate_gir(
    libchamplain_gtk_sha,
    sources: libchamplain_gtk_sources + libchamplain_gtk_public_h,
    nsversion: api_version,
    namespace: 'GtkChamplain',
    symbol_prefix: 'gtk_champlain',
    identifier_prefix: 'GtkChamplain',
    header: 'champlain-gtk/champlain-gtk.h',
    export_packages: [libchamplain_gtk_name],
    includes: ['Clutter-1.0', 'Gtk-3.0', libchamplain_gir.get(0)],
    link_with: libchamplain_gtk_sha,
    install: true,
    install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
    extra_args: [
      '-DCHAMPLAIN_GTK_COMPILATION',
    ]
  )

  libchamplain_gtk_dep_sources += [
    libchamplain_gtk_gir,
  ]

  if generate_vapi
    libchamplain_gtk_vapi_packages = [
      'clutter-gtk-1.0',
      'cogl-pango-1.0',
      'gtk+-3.0',
      'atk',
      'pangocairo',
      libchamplain_vapi,
    ]

    libchamplain_gtk_vapi = gnome.generate_vapi(
      libchamplain_gtk_name,
      sources: libchamplain_gtk_gir.get(0),
      packages: libchamplain_gtk_vapi_packages,
      install: true,
      install_dir: vapidir,
    )
  endif
endif

libchamplain_gtk_dep = declare_dependency(
  link_with: libchamplain_gtk_sha,
  include_directories: rootdir,
  dependencies: libchamplain_gtk_deps,
  sources: libchamplain_gtk_dep_sources,
)

libchamplain_gtk_pc = pkg.generate(
  libchamplain_gtk_sha,
  description: 'Gtk+ Widget wrapper for libchamplain',
  subdirs: package_gtk_string,
  install_dir: pkgconfigdir,
  requires: libchamplain_gtk_requires,
)
