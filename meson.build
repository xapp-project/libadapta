project('libadapta', 'c',
          version: '1.5.8',
          license: 'LGPL-2.1-or-later',
    meson_version: '>= 0.59.0',
  default_options: [ 'warning_level=1', 'buildtype=debugoptimized', 'c_std=gnu11' ],
)

version_arr = meson.project_version().split('-')[0].split('.')
adapta_version_major = version_arr[0].to_int()
adapta_version_minor = version_arr[1].to_int()
adapta_version_short = '@0@.@1@'.format(adapta_version_major, adapta_version_minor)

if version_arr[2].startswith('alpha') or version_arr[2].startswith('beta') or version_arr[2].startswith('rc')
  adapta_version_micro = 0
else
  adapta_version_micro = version_arr[2].to_int()
endif

# The major api version as encoded in the libraries name
apiversion = adapta_version_major.to_string()
package_api_name = '@0@-@1@'.format(meson.project_name(), apiversion)

# Before making a release, the libversion string should be modified.
#
#  * Bump the first component if binary compatibility has been broken; or
#  * Bump the second component if new APIs are added; or
#  * Bump the third component otherwise.
#
# When bumping the first component version, set the second and third components
# to 0. When bumping the second version, set the third one to zero.
#
# A lot easier than libtool, right?
libversion = '0.5.8'
# The so major version of the library
soversion = 0

add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-DADAPTA_COMPILATION',
], language: 'c')

root_inc = include_directories('.')
src_inc = include_directories('src')

cc = meson.get_compiler('c')

global_c_args = []
test_c_args = []

target_system = target_machine.system()

has_winrt = target_system == 'windows' \
  and cc.check_header('roapi.h') \
  and cc.check_header('Windows.Foundation.h') \
  and cc.check_header('Windows.UI.ViewManagement.h')

if has_winrt
  add_project_arguments(['-DHAS_WINRT'], language: 'c')
endif

if cc.get_id() != 'msvc'
  test_c_args += [
    '-Wcast-align',
    '-Wdate-time',
    ['-Werror=format-security', '-Werror=format=2'],
    '-Wendif-labels',
    '-Werror=incompatible-pointer-types',
    '-Werror=missing-declarations',
    '-Werror=overflow',
    '-Werror=return-type',
    '-Werror=shift-count-overflow',
    '-Werror=shift-overflow=2',
    '-Werror=implicit-fallthrough=3',
    '-Wformat-nonliteral',
    '-Wformat-security',
    '-Winit-self',
    '-Wmaybe-uninitialized',
    '-Wmissing-field-initializers',
    '-Wmissing-include-dirs',
    '-Wmissing-noreturn',
    '-Wnested-externs',
    '-Wno-missing-field-initializers',
    '-Wno-sign-compare',
    '-Wno-strict-aliasing',
    '-Wno-unused-parameter',
    '-Wold-style-definition',
    '-Wpointer-arith',
    '-Wredundant-decls',
    '-Wshadow',
    '-Wstrict-prototypes',
    '-Wswitch-default',
    '-Wswitch-enum',
    '-Wtype-limits',
    '-Wundef',
    '-Wunused-function',
    '-Wfloat-equal',
  ]

  if get_option('buildtype') != 'plain'
    if target_system == 'windows'
      test_c_args += '-fstack-protector'
    else
      test_c_args += '-fstack-protector-strong'
    endif
  endif
  if get_option('profiling')
    test_c_args += '-pg'
  endif
else
  test_c_args += '-FImsvc_recommended_pragmas.h'
endif

foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    global_c_args += arg
  endif
endforeach
add_project_arguments(
  global_c_args,
  language: 'c'
)

# Setup various paths that subdirectory meson.build files need
package_subdir = get_option('package_subdir') # When used as subproject
if package_subdir != ''
  datadir    = get_option('datadir') / package_subdir
  libdir     = get_option('libdir')  / package_subdir
  girdir     = get_option('datadir') / package_subdir / 'gir-1.0'
  theme_dir  = get_option('datadir') / package_subdir / 'themes' / 'LibAdapta-Example' / 'libadapta-@0@'.format(adapta_version_short)
  typelibdir = get_option('libdir')  / package_subdir / 'girepository-1.0'
  vapidir    = get_option('datadir') / package_subdir / 'vapi'

else
  datadir    = get_option('datadir')
  libdir     = get_option('libdir')
  girdir     = get_option('datadir') / 'gir-1.0'
  theme_dir  = get_option('datadir') / 'themes' / 'LibAdapta-Example' / 'libadapta-@0@'.format(adapta_version_short)
  typelibdir = get_option('libdir')  / 'girepository-1.0'
  vapidir    = get_option('datadir') / 'vala' / 'vapi'
endif

introspection_feature = get_option('introspection')
introspection = introspection_feature.enabled() or introspection_feature.auto()

gnome = import('gnome')

subdir('src')
subdir('po')
subdir('demo')
subdir('examples')
subdir('tests')
subdir('doc')

run_data = configuration_data()
run_data.set('ABS_BUILDDIR', meson.current_build_dir())
run_data.set('ABS_SRCDIR', meson.current_source_dir())
configure_file(
          input: 'run.in',
         output: 'run',
  configuration: run_data)

summary(
  {
    'Tests': get_option('tests'),
    'Examples': get_option('examples'),
    'Documentation': get_option('gtk_doc'),
    'Introspection': introspection,
    'Vapi': get_option('vapi'),
  }, section: 'Options')
