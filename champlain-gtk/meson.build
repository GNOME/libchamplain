
headers_subdir = meson.project_name() + '-gtk-' + api_version
headers_subdir_full = join_paths(headers_subdir, 'champlain-gtk')

libchamplain_gtk_headers_public = files(
  'gtk-champlain-embed.h',
  'champlain-gtk.h'
)

install_headers(
  libchamplain_gtk_headers_public,
  subdir: headers_subdir_full
)

libchamplain_gtk_sources = files(
  'gtk-champlain-embed.c'
)

champlain_gtk_marshal = gnome.genmarshal('champlain-gtk-marshal',
  sources: 'champlain-gtk-marshal.list',
  prefix: '_champlain_gtk_marshal',
  install_header: true,
  install_dir: join_paths(get_option('includedir'), headers_subdir_full)
)

champlain_gtk_enum_types = gnome.mkenums_simple('champlain-gtk-enum-types',
  sources: libchamplain_gtk_headers_public,
  install_header: true,
  install_dir: join_paths(get_option('includedir'), headers_subdir_full)
)

libchamplain_gtk_library_sources = [
  libchamplain_gtk_headers_public,
  libchamplain_gtk_sources,
  champlain_gtk_marshal,
  champlain_gtk_enum_types
]

libchamplain_gtk_library = shared_library('champlain-gtk',
  libchamplain_gtk_library_sources,
  dependencies: libchamplain_gtk_deps,
  include_directories: root_dir,
  c_args: [
    '-DCHAMPLAIN_GTK_COMPILATION',
    '-DG_LOG_DOMAIN="@0@"'.format (meson.project_name() + '-gtk')
  ],
  install: true
)

libchamplain_gtk_pkg = ['clutter-gtk-1.0', 'champlain-' + api_version, 'gtk+-3.0']
libchamplain_gtk_gir_includes = ['Clutter-1.0', 'Gtk-3.0', libchamplain_gir[0]]

libchamplain_gtk_gir = gnome.generate_gir(
  libchamplain_gtk_library,
  sources: libchamplain_gtk_library_sources,
  dependencies: libchamplain_gtk_deps,
  header: 'champlain-gtk/champlain-gtk.h',
  namespace: 'GtkChamplain',
  nsversion: api_version,
  symbol_prefix: 'gtk_champlain',
  includes: libchamplain_gtk_gir_includes,
  extra_args: [ '-DCHAMPLAIN_GTK_COMPILATION' ],
  install: true
)

libchamplain_gtk_pkg_name = 'champlain-gtk-' + api_version
if get_option('vala-support')
  libchamplain_gtk_vapi = gnome.generate_vapi(
    libchamplain_gtk_pkg_name,
    sources: libchamplain_gtk_gir[0],
    packages: libchamplain_gtk_pkg,
    install: true
  )
endif

libchamplain_gtk_dir = include_directories('.')
libchamplain_gtk_dep = declare_dependency(
  link_with: libchamplain_gtk_library,
  include_directories: [root_dir, libchamplain_gtk_dir],
  dependencies: libchamplain_gtk_deps
)

pkg.generate(
  filebase: libchamplain_gtk_pkg_name,
  name: 'libchamplain-gtk',
  description: 'Gtk+ Widget wrapper for libchamplain',
  version: meson.project_version(),
  libraries: libchamplain_gtk_library,
  requires: libchamplain_gtk_pkg,
  subdirs: 'libchamplain-gtk-' + api_version
)