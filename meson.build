project(
  'libchamplain', 'c',
  version: '0.12.19',
  license: 'LGPLv2.1+',
  meson_version: '>= 0.49.0',
  default_options: [
    'buildtype=debugoptimized',
  ]
)

gnome = import('gnome')
pkg = import('pkgconfig')

# Versioning
version = meson.project_version()
version_arr = version.split('.')
version_major = version_arr[0].to_int()
version_minor = version_arr[1].to_int()
version_micro = version_arr[2].to_int()

api_version = '@0@.@1@'.format(
  version_major,
  version_minor.is_odd() ? version_minor + 1 : version_minor,
)

# Before making a release, the lt_version string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
lt_version = '11:8:11'

lt_arr = lt_version.split(':')
lib_version = '@0@.@1@.@2@'.format(lt_arr[0].to_int() - lt_arr[2].to_int(), lt_arr[2], lt_arr[1])

package_string = '@0@-@1@'.format(meson.project_name(), api_version)
package_gtk_string = '@0@-gtk-@1@'.format(meson.project_name(), api_version)

# Paths
rootdir = include_directories('.')

prefix = get_option('prefix')
includedir = join_paths(prefix, get_option('includedir'))
libdir = join_paths(prefix, get_option('libdir'))
datadir = join_paths(prefix, get_option('datadir'))

# Dependencies
cc = meson.get_compiler('c')

libm_dep = cc.find_library('m', required: true)

glib_req = '>= 2.38'
gtk_req = '>= 3.0'
clutter_req = '>= 1.24'
clutter_gtk_req = '>= 1.0'
cairo_req = '>= 1.4'
sqlite_req = '>= 3.0'
libsoup_req = '>= 2.42'
memphis_req = '>= 0.2.1'
introspection_req = '>= 0.6.3'
vala_req = '>= 0.11.0'
gtk_doc_req = '>= 1.15'

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

build_gtk_widgetry = get_option('widgetry')
if build_gtk_widgetry and not (gtk_dep.found() and clutter_gtk_dep.found())
  build_gtk_widgetry = false
endif

build_gtk_doc = get_option('gtk_doc')
if build_gtk_doc and not (gtk_doc_dep.found() and build_gtk_widgetry)
  build_gtk_doc = false
endif

generate_gir = get_option('introspection')
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
if build_gtk_widgetry
  subdir('champlain-gtk')
endif

subdir('docs')

if build_demos
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
