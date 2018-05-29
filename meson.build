project(
  'libchamplain', 'c',
  version: '0.12.17',
  license: 'LGPLv2.1+',
  meson_version: '>= 0.46.1',
  default_options: [
    'buildtype=debugoptimized',
  ]
)

gnome = import('gnome')
pkg = import('pkgconfig')

# Versioning
version = meson.project_version()
version_arr = version.split('.')
version_major = version_arr[0]
version_minor = version_arr[1]
version_micro = version_arr[2]

if version_minor.to_int().is_odd()
  api_version = '.'.join([version_major, version_minor + 1])
else
  api_version = '.'.join([version_major, version_minor])
endif

lib_version = '0.10.0'

package_name = meson.project_name().strip('lib')
package_string = '@0@-@1@'.format(package_name, api_version)
package_gtk_name = '@0@-gtk'.format(package_name)
package_gtk_string = '@0@-@1@'.format(package_gtk_name, api_version)

# Paths
rootdir = include_directories('.')

srcdir = meson.current_source_dir()
builddir = meson.current_build_dir()

prefix = get_option('prefix')

includedir = join_paths(prefix, get_option('includedir'))
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))
pkgincludedir = join_paths(includedir, package_string)
typelibdir = join_paths(libdir, 'girepository-1.0')
pkgconfigdir = join_paths(libdir, 'pkgconfig')

girdir = join_paths(datadir, 'gir-1.0')
gtkdocdir = join_paths(datadir, 'gtk-doc')
vapidir = join_paths(datadir, 'vala', 'vapi')

# Dependencies
cc = meson.get_compiler('c')

libm_dep = cc.find_library('m', required: true)

glib_req = '>= 2.16.0'
gtk_req = '>= 2.90.0'
clutter_req = '>= 1.12'
clutter_gtk_req = '>= 0.90'
cairo_req = '>= 1.4'
sqlite_req = '>= 1.12.0'
libsoup_req = '>= 2.42'
memphis_req = '>=0.2.1'
introspection_req = '>= 0.6.3'
vala_req = '>= 0.11.0'
gtk_doc_req = '>= 1.9'

glib_dep = dependency('glib-2.0', version: glib_req)
gobject_dep = dependency('gobject-2.0', version: glib_req)
gio_dep = dependency('gio-2.0', version: glib_req)
clutter_dep = dependency('clutter-1.0', version: clutter_req)
cairo_dep = dependency('cairo', version: cairo_req)
sqlite_dep = dependency('sqlite3', version: sqlite_req)
libsoup_dep = dependency('libsoup-2.4', version: libsoup_req)

gtk_dep = dependency('gtk+-3.0', version: gtk_req, required: false)
clutter_gtk_dep = dependency('clutter-gtk-1.0', version: clutter_gtk_req, required: false)
memphis_dep = dependency('memphis-0.2', version: memphis_req, required: false)

introspection_dep = dependency('gobject-introspection-1.0', version: introspection_req, required: false)
vapigen_dep = dependency('vapigen', version: vala_req, required: false)
gtk_doc_dep = dependency('gtk-doc', version: gtk_doc_req, required: false)

# Configurations
config_h = configuration_data()
config_h.set_quoted('VERSION', version)

configure_file(
  output: 'config.h',
  configuration: config_h
)

# Options
build_demos = get_option('demos')

build_gtk_doc = get_option('gtk_doc')
if build_gtk_doc and not gtk_doc_dep.found()
  build_gtk_doc = false
endif

build_gtk_widgetry = get_option('widgetry')
if build_gtk_widgetry and not gtk_doc_dep.found()
  build_gtk_widgetry = false
elif build_gtk_widgetry and not clutter_gtk_dep.found()
  build_gtk_widgetry = false
endif

generate_gir = get_option('gir')
if generate_gir and not introspection_dep.found()
  generate_gir = false
endif

build_with_memphis = get_option('memphis')
if build_with_memphis and not memphis_dep.found()
  build_with_memphis = false
endif

generate_vapi = generate_gir and get_option('vapi')
if generate_vapi and not vapigen_dep.found()
  generate_vapi = false
endif

subdir('champlain')
if build_gtk_widgetry == true
  subdir('champlain-gtk')
endif

subdir('docs')

if build_demos == true
  subdir('demos')
endif

summary = [
  '',
  '------',
  'libchamplain @0@ (@1@)'.format(version, api_version),
  '',
  '             Demos: @0@'.format(build_demos),
  '     Documentation: @0@'.format(build_gtk_doc),
  '     GTK+ widgetry: @0@'.format(build_gtk_widgetry),
  '     Introspection: @0@'.format(generate_gir),
  '  Memphis renderer: @0@'.format(build_with_memphis),
  '          Vala API: @0@'.format(generate_vapi),
  '',
  'Directories:',
  '           prefix: @0@'.format(prefix),
  '       includedir: @0@'.format(includedir),
  '           libdir: @0@'.format(libdir),
  '          datadir: @0@'.format(datadir),
  '------',
]

message('\n'.join(summary))