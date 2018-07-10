project('libchamplain', 'c',
  version: '0.12.17',
  meson_version: '>=0.46.0'
)

version_parts = meson.project_version().split('.')
major_version = version_parts[0]
minor_version = version_parts[1]
micro_version = version_parts[2]

if minor_version.to_int().is_odd()
  api_version = '.'.join(major_version, minor_version.to_int() + 1)
else
  api_version = '.'.join([major_version, minor_version])
endif

gnome = import('gnome')
pkg = import('pkgconfig')

gio_dep = dependency('gio-2.0', version: '>=2.16')
glib_dep = dependency('glib-2.0', version: '>=2.16')
gobject_dep = dependency('gobject-2.0', version: '>=2.10')
gdk_dep = dependency('gdk-3.0', version: '>=2.90')
clutter_dep = dependency('clutter-1.0', version: '>=1.12')
cairo_dep = dependency('cairo', version: '>=1.4')
sqlite3_dep = dependency('sqlite3', version: '>=3.0')
libsoup_dep = dependency('libsoup-2.4', version: '>=2.42')
m_dep = meson.get_compiler('c').find_library('m', required : false)

libchamplain_deps = [
  gio_dep,
  glib_dep,
  gobject_dep,
  gdk_dep,
  clutter_dep,
  cairo_dep,
  sqlite3_dep,
  libsoup_dep,
  m_dep
]

config_data = configuration_data()
config_h = configure_file(
  output: 'config.h',
  configuration: config_data
)

if get_option('debug')
  add_project_arguments('-DENABLE_DEBUG=1', language: 'c')
endif

if get_option('memphis')
  memphis_dep = dependency('memphis-0.2', version: '>=0.2.1')
  add_project_arguments('-DCHAMPLAIN_HAS_MEMPHIS=1', language: 'c')
  libchamplain_deps += memphis_dep
endif

root_dir = include_directories('.')

subdir('champlain')

if get_option('gtk')
  gtk_dep = dependency('gtk+-3.0', version: '>=2.90')
  clutter_gtk_dep = dependency('clutter-gtk-1.0', version: '>=0.90')

  libchamplain_gtk_deps = [
    libchamplain_dep,
    gtk_dep,
    clutter_gtk_dep
  ]

  subdir('champlain-gtk')
endif

subdir('demos')
subdir('docs')