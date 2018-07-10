
version_data = configuration_data()
version_data.set('CHAMPLAIN_VERSION', meson.project_version())
version_data.set('CHAMPLAIN_MAJOR_VERSION', major_version)
version_data.set('CHAMPLAIN_MINOR_VERSION', minor_version)
version_data.set('CHAMPLAIN_MICRO_VERSION', micro_version)

headers_subdir = meson.project_name() + '-' + api_version
headers_subdir_full = join_paths(headers_subdir, 'champlain')

champlain_version_h = configure_file(
  input: 'champlain-version.h.in',
  output: '@BASENAME@',
  configuration: version_data,
  install_dir: join_paths(get_option('includedir'), headers_subdir_full)
)

feature_data = configuration_data()
if get_option('memphis')
  feature_data.set('CHAMPLAIN_HAS_MEMPHIS', '1')
endif

champlain_features_h = configure_file(
  output: 'champlain-features.h',
  configuration: feature_data,
  install_dir: join_paths(get_option('includedir'), headers_subdir_full)
)

libchamplain_headers_public = files(
  'champlain.h',
  'champlain-defines.h',
  'champlain-point.h',
  'champlain-custom-marker.h',
  'champlain-view.h',
  'champlain-layer.h',
  'champlain-marker-layer.h',
  'champlain-path-layer.h',
  'champlain-location.h',
  'champlain-coordinate.h',
  'champlain-marker.h',
  'champlain-label.h',
  'champlain-scale.h',
  'champlain-license.h',
  'champlain-tile.h',
  'champlain-map-source.h',
  'champlain-map-source-chain.h',
  'champlain-tile-source.h',
  'champlain-tile-cache.h',
  'champlain-memory-cache.h',
  'champlain-network-tile-source.h',
  'champlain-file-cache.h',
  'champlain-map-source-factory.h',
  'champlain-map-source-desc.h',
  'champlain-renderer.h',
  'champlain-image-renderer.h',
  'champlain-error-tile-renderer.h',
  'champlain-file-tile-source.h',
  'champlain-null-tile-source.h',
  'champlain-network-bbox-tile-source.h',
  'champlain-adjustment.h',
  'champlain-kinetic-scroll-view.h',
  'champlain-viewport.h',
  'champlain-bounding-box.h',
  'champlain-exportable.h'
)

if get_option('memphis')
  libchamplain_headers_public += files('champlain-memphis-renderer.h')
endif

install_headers(
  libchamplain_headers_public,
  subdir: headers_subdir_full
)

libchamplain_headers_private = files(
  'champlain-debug.h',
  'champlain-private.h'
)
libchamplain_headers_private += config_h

libchamplain_sources = files(
  'champlain-debug.c',
  'champlain-view.c',
  'champlain-layer.c',
  'champlain-marker-layer.c',
  'champlain-path-layer.c',
  'champlain-location.c',
  'champlain-coordinate.c',
  'champlain-marker.c',
  'champlain-label.c',
  'champlain-scale.c',
  'champlain-license.c',
  'champlain-tile.c',
  'champlain-map-source.c',
  'champlain-map-source-chain.c',
  'champlain-tile-source.c',
  'champlain-tile-cache.c',
  'champlain-memory-cache.c',
  'champlain-network-tile-source.c',
  'champlain-file-cache.c',
  'champlain-map-source-factory.c',
  'champlain-map-source-desc.c',
  'champlain-point.c',
  'champlain-custom-marker.c',
  'champlain-renderer.c',
  'champlain-image-renderer.c',
  'champlain-error-tile-renderer.c',
  'champlain-file-tile-source.c',
  'champlain-null-tile-source.c',
  'champlain-network-bbox-tile-source.c',
  'champlain-adjustment.c',
  'champlain-kinetic-scroll-view.c',
  'champlain-viewport.c',
  'champlain-bounding-box.c',
  'champlain-exportable.c'
)

if get_option('memphis')
  libchamplain_sources += files('champlain-memphis-renderer.c')
endif

champlain_marshal = gnome.genmarshal('champlain-marshal',
  sources: 'champlain-marshal.list',
  prefix: '_champlain_marshal',
  install_header: true,
  install_dir: join_paths(get_option('includedir'), headers_subdir_full)
)

champlain_enum_types = gnome.mkenums_simple('champlain-enum-types',
  sources: libchamplain_headers_public,
  install_header: true,
  install_dir: join_paths(get_option('includedir'), headers_subdir_full)
)

libchamplain_library = shared_library('champlain',
  libchamplain_headers_public,
  champlain_version_h,
  champlain_features_h,
  libchamplain_headers_private,
  libchamplain_sources,
  champlain_marshal,
  champlain_enum_types,
  dependencies: libchamplain_deps,
  include_directories: root_dir,
  c_args: [
    '-DCHAMPLAIN_COMPILATION',
    '-DG_LOG_DOMAIN="@0@"'.format (meson.project_name())
  ],
  install: true
)

libchamplain_pkg = ['clutter-1.0', 'cogl-pango-1.0', 'atk', 'pangocairo']
libchamplain_gir_includes = ['GObject-2.0', 'Clutter-1.0']

if get_option('memphis')
  libchamplain_gir_includes += 'Memphis-0.2'
endif

libchamplain_gir = gnome.generate_gir(
  libchamplain_library,
  sources: [
    libchamplain_headers_public,
    champlain_version_h,
    champlain_enum_types,
    champlain_marshal,
    libchamplain_sources
  ],
  dependencies: libchamplain_deps,
  header: 'champlain/champlain.h',
  namespace: 'Champlain',
  nsversion: api_version,
  symbol_prefix: 'champlain',
  includes: libchamplain_gir_includes,
  extra_args: [ '-DCHAMPLAIN_COMPILATION' ],
  install: true
)

libchamplain_pkg_name = 'champlain-' + api_version
if get_option('vala-support')
  libchamplain_vapi = gnome.generate_vapi(
    libchamplain_pkg_name,
    sources: libchamplain_gir[0],
    packages: libchamplain_pkg,
    install: true
  )
endif

libchamplain_dir = include_directories('.')
libchamplain_dep = declare_dependency(
  link_with: libchamplain_library,
  include_directories: [ root_dir, libchamplain_dir ],
  dependencies: libchamplain_deps
)

pkg.generate(
  filebase: libchamplain_pkg_name,
  name: 'libchamplain',
  description: 'Map View for Clutter',
  version: meson.project_version(),
  libraries: libchamplain_library,
  requires: ['gobject-2.0', 'glib-2.0', 'clutter-1.0'],
  requires_private: ['gdk-3.0', 'gio-2.0', 'cairo', 'sqlite3', 'libsoup-2.4'],
  subdirs: 'libchamplain-' + api_version
)
