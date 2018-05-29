libchamplain_gtk_public_h = [
  'champlain-gtk.h',
  'gtk-champlain-embed.h',
]

libchamplain_gtk_private_h = [
]

libchamplain_gtk_sources = [
  'gtk-champlain-embed.c',
]

libchamplain_gtk_deps = [
  glib_dep,
  gobject_dep,
  gtk_dep,
  clutter_gtk_dep,
  libchamplain_dep,
]

libchamplain_gtk_c_args = [
  '-DHAVE_CONFIG_H',
  '-DCHAMPLAIN_GTK_COMPILATION',
  '-DG_LOG_DOMAIN="@0@"'.format(package_gtk_name),
]

libchamplain_gtk_link_args = [
]

libchamplain_gtk_marshals = gnome.genmarshal(
  'champlain-gtk-marshal',
  sources: 'champlain-gtk-marshal.list',
  prefix: '_champlain_gtk_marshal',
  install_header: true,
  install_dir: join_paths(
    pkgincludedir,
    'champlain-gtk',
  )
)

libchamplain_gtk_enums = gnome.mkenums(
  'champlain-gtk-enum-types',
  sources: libchamplain_gtk_public_h,
  h_template: 'champlain-gtk-enum-types.h.in',
  c_template: 'champlain-gtk-enum-types.c.in',
  install_header: true,
  install_dir: join_paths(
    pkgincludedir,
    'champlain-gtk',
  )
)

libchamplain_gtk_marshals_h = libchamplain_gtk_marshals.get(1)
libchamplain_gtk_enums_h = libchamplain_gtk_enums.get(1)

libchamplain_gtk_sources += [
  libchamplain_gtk_marshals,
  libchamplain_gtk_enums,
]

install_headers(
  libchamplain_gtk_public_h,
  install_dir: join_paths(
    pkgincludedir,
    'champlain-gtk',
  )
)

libchamplain_gtk_sha = shared_library(
  package_gtk_string,
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
  libchamplain_gtk_enums_h,
  libchamplain_gtk_marshals_h,
]

if generate_gir == true
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
    export_packages: [package_gtk_string],
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

  if generate_vapi == true and libchamplain_gtk_gir.length() > 0
    libchamplain_gtk_vapi_packages = [
      'clutter-gtk-1.0',
      'cogl-pango-1.0',
      'gtk+-3.0',
      'atk',
      'pangocairo',
      libchamplain_vapi,
    ]

    libchamplain_gtk_vapi = gnome.generate_vapi(
      package_gtk_string,
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
  name: package_gtk_string,
  description: 'Gtk+ Widget wrapper for libchamplain',
  requires: ['gtk+-3.0', 'clutter-gtk-1.0', package_string],
  version: version,
  subdirs: package_string,
  libraries: libchamplain_gtk_sha,
  install_dir: pkgconfigdir,
)