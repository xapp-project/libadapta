libadapta_header_subdir = package_subdir / package_api_name
libadapta_header_dir = get_option('includedir') / libadapta_header_subdir

subdir('stylesheet')

libadapta_resources = gnome.compile_resources(
  'adap-resources',
  'adapta.gresources.xml',
  source_dir: meson.current_source_dir(),
  c_name: 'adap',
)

adap_public_enum_headers = [
  'adap-alert-dialog.h',
  'adap-animation.h',
  'adap-banner.h',
  'adap-breakpoint.h',
  'adap-dialog.h',
  'adap-flap.h',
  'adap-fold-threshold-policy.h',
  'adap-easing.h',
  'adap-header-bar.h',
  'adap-leaflet.h',
  'adap-length-unit.h',
  'adap-navigation-direction.h',
  'adap-style-manager.h',
  'adap-squeezer.h',
  'adap-tab-view.h',
  'adap-toast.h',
  'adap-toolbar-view.h',
  'adap-view-switcher.h',
]

adap_private_enum_headers = [
  'adap-settings-private.h',
]

version_data = configuration_data()
version_data.set('ADAP_MAJOR_VERSION', adapta_version_major)
version_data.set('ADAP_MINOR_VERSION', adapta_version_minor)
version_data.set('ADAP_MICRO_VERSION', adapta_version_micro)
version_data.set('ADAP_VERSION', meson.project_version())

adap_version_h = configure_file(
           input: 'adap-version.h.in',
          output: 'adap-version.h',
     install_dir: libadapta_header_dir,
   configuration: version_data)

libadapta_generated_headers = [
  adap_version_h,
]

install_headers(['adapta.h'],
  subdir: libadapta_header_subdir)

# Filled out in the subdirs
libadapta_public_headers = []
libadapta_public_sources = []
libadapta_private_sources = []

adap_public_enums = gnome.mkenums_simple('adap-enums',
         sources: adap_public_enum_headers,
     body_prefix: '#include "config.h"',
   header_prefix: '#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)\n' +
                  '#error "Only <adapta.h> can be included directly."\n' +
                  '#endif\n' +
                  '\n' +
                  '#include "adap-version.h"\n',
       decorator: 'ADAP_AVAILABLE_IN_ALL',
  install_header: true,
     install_dir: libadapta_header_dir,
)

adap_private_enums = gnome.mkenums_simple('adap-enums-private',
         sources: adap_private_enum_headers,
     body_prefix: '#include "config.h"',
   header_prefix: '#if !defined(_ADAPTA_INSIDE) && !defined(ADAPTA_COMPILATION)\n' +
                  '#error "Only <adapta.h> can be included directly."\n' +
                  '#endif\n' +
                  '\n' +
                  '#include "adap-enums.h"\n',
  install_header: false,
)

libadapta_public_sources += [adap_public_enums[0]]
libadapta_private_sources += adap_private_enums
libadapta_generated_headers += [adap_public_enums[1]]

adap_marshalers = gnome.genmarshal('adap-marshalers',
  sources: 'adap-marshalers.list',
  prefix: 'adap_marshal',
  valist_marshallers: true,
)

libadapta_private_sources += adap_marshalers

src_headers = [
  'adap-about-dialog.h',
  'adap-about-window.h',
  'adap-action-row.h',
  'adap-alert-dialog.h',
  'adap-animation.h',
  'adap-animation-target.h',
  'adap-animation-util.h',
  'adap-application.h',
  'adap-application-window.h',
  'adap-avatar.h',
  'adap-banner.h',
  'adap-bin.h',
  'adap-breakpoint.h',
  'adap-breakpoint-bin.h',
  'adap-button-content.h',
  'adap-carousel.h',
  'adap-carousel-indicator-dots.h',
  'adap-carousel-indicator-lines.h',
  'adap-clamp.h',
  'adap-clamp-layout.h',
  'adap-clamp-scrollable.h',
  'adap-combo-row.h',
  'adap-dialog.h',
  'adap-easing.h',
  'adap-entry-row.h',
  'adap-enum-list-model.h',
  'adap-expander-row.h',
  'adap-flap.h',
  'adap-fold-threshold-policy.h',
  'adap-header-bar.h',
  'adap-leaflet.h',
  'adap-length-unit.h',
  'adap-main.h',
  'adap-message-dialog.h',
  'adap-navigation-direction.h',
  'adap-navigation-split-view.h',
  'adap-navigation-view.h',
  'adap-overlay-split-view.h',
  'adap-password-entry-row.h',
  'adap-preferences-dialog.h',
  'adap-preferences-group.h',
  'adap-preferences-page.h',
  'adap-preferences-row.h',
  'adap-preferences-window.h',
  'adap-spin-row.h',
  'adap-split-button.h',
  'adap-spring-animation.h',
  'adap-spring-params.h',
  'adap-squeezer.h',
  'adap-status-page.h',
  'adap-style-manager.h',
  'adap-swipe-tracker.h',
  'adap-swipeable.h',
  'adap-switch-row.h',
  'adap-tab-bar.h',
  'adap-tab-button.h',
  'adap-tab-overview.h',
  'adap-tab-view.h',
  'adap-timed-animation.h',
  'adap-toast.h',
  'adap-toast-overlay.h',
  'adap-toolbar-view.h',
  'adap-view-stack.h',
  'adap-view-switcher.h',
  'adap-view-switcher-bar.h',
  'adap-view-switcher-title.h',
  'adap-window.h',
  'adap-window-title.h',
]

gen_public_types = find_program('gen-public-types.py', required: true)

libadapta_init_public_types = custom_target('adap-public-types.c',
   output: 'adap-public-types.c',
    input: [src_headers, libadapta_generated_headers],
  command: [gen_public_types, '@INPUT@'],
  capture: true,
)

src_sources = [
  'adap-about-dialog.c',
  'adap-about-window.c',
  'adap-action-row.c',
  'adap-alert-dialog.c',
  'adap-animation.c',
  'adap-animation-target.c',
  'adap-animation-util.c',
  'adap-application.c',
  'adap-application-window.c',
  'adap-avatar.c',
  'adap-banner.c',
  'adap-bin.c',
  'adap-breakpoint.c',
  'adap-breakpoint-bin.c',
  'adap-button-content.c',
  'adap-carousel.c',
  'adap-carousel-indicator-dots.c',
  'adap-carousel-indicator-lines.c',
  'adap-clamp.c',
  'adap-clamp-layout.c',
  'adap-clamp-scrollable.c',
  'adap-combo-row.c',
  'adap-dialog.c',
  'adap-easing.c',
  'adap-entry-row.c',
  'adap-enum-list-model.c',
  'adap-expander-row.c',
  'adap-flap.c',
  'adap-fold-threshold-policy.c',
  'adap-header-bar.c',
  'adap-leaflet.c',
  'adap-length-unit.c',
  'adap-main.c',
  'adap-message-dialog.c',
  'adap-navigation-direction.c',
  'adap-navigation-split-view.c',
  'adap-navigation-view.c',
  'adap-overlay-split-view.c',
  'adap-password-entry-row.c',
  'adap-preferences-dialog.c',
  'adap-preferences-group.c',
  'adap-preferences-page.c',
  'adap-preferences-row.c',
  'adap-preferences-window.c',
  'adap-spin-row.c',
  'adap-split-button.c',
  'adap-spring-animation.c',
  'adap-spring-params.c',
  'adap-squeezer.c',
  'adap-style-manager.c',
  'adap-status-page.c',
  'adap-swipe-tracker.c',
  'adap-swipeable.c',
  'adap-switch-row.c',
  'adap-tab-bar.c',
  'adap-tab-button.c',
  'adap-tab-overview.c',
  'adap-tab-view.c',
  'adap-timed-animation.c',
  'adap-toast.c',
  'adap-toast-overlay.c',
  'adap-toolbar-view.c',
  'adap-version.c',
  'adap-view-stack.c',
  'adap-view-switcher.c',
  'adap-view-switcher-bar.c',
  'adap-view-switcher-title.c',
  'adap-window.c',
  'adap-window-title.c',
]

# Files that should not be introspected
libadapta_private_sources += files([
  'adap-back-button.c',
  'adap-bidi.c',
  'adap-bottom-sheet.c',
  'adap-dialog-host.c',
  'adap-fading-label.c',
  'adap-floating-sheet.c',
  'adap-gizmo.c',
  'adap-gtkbuilder-utils.c',
  'adap-indicator-bin.c',
  'adap-inspector-page.c',
  'adap-settings.c',
  'adap-settings-impl.c',
  'adap-settings-impl-gsettings.c',
  'adap-settings-impl-legacy.c',
  'adap-sheet-controls.c',
  'adap-shadow-helper.c',
  'adap-tab.c',
  'adap-tab-box.c',
  'adap-tab-grid.c',
  'adap-tab-thumbnail.c',
  'adap-toast-widget.c',
  'adap-view-switcher-button.c',
  'adap-widget-utils.c',
])

libadapta_public_headers += files(src_headers)
libadapta_public_sources += files(src_sources)

install_headers(src_headers, subdir: libadapta_header_subdir)
install_headers(['adw-compat.h'], subdir: libadapta_header_subdir)

glib_min_version = '>= 2.76.0'
gtk_min_version = '>= 4.13.4'

gio_dep = dependency('gio-2.0', version: glib_min_version)
gtk_dep = dependency('gtk4', version: gtk_min_version)
appstream_dep = dependency('appstream',
  fallback : ['appstream', 'appstream_dep'],
  default_options : [
    'systemd=false', 'apidocs=false', 'install-docs=false',
    'stemming=false', 'svg-support=false', 'gir=false',
  ],
)

libadapta_deps = [
  dependency('glib-2.0', version: glib_min_version),
  dependency('fribidi'),
  gio_dep,
  gtk_dep,
  appstream_dep,
  cc.find_library('m', required: false),
]

libadapta_public_deps = [
  gio_dep,
  gtk_dep,
]

libadapta_c_args = [
  '-DG_LOG_DOMAIN="Adapta"',
]

config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', 'libadapta')
config_h.set_quoted('LOCALEDIR', get_option('prefix') / get_option('localedir'))

# Symbol visibility
if target_system == 'windows'
  config_h.set('DLL_EXPORT', true)
  config_h.set('_ADAP_EXTERN', '__declspec(dllexport) extern')
else
  config_h.set('_ADAP_EXTERN', '__attribute__((visibility("default"))) extern')
endif

if target_system == 'darwin'
  appleframework_modules = [
    'AppKit',
    'Foundation',
  ]
  libadapta_deps += [
    dependency('appleframeworks', modules: appleframework_modules),
  ]
  libadapta_private_sources += [
    'adap-settings-impl-macos.c',
  ]
  libadapta_c_args += ['-xobjective-c']
elif target_system == 'windows'
  libadapta_deps += [
    dependency('gtk4-win32', version: gtk_min_version),
  ]
  libadapta_private_sources += [
    'adap-settings-impl-win32.c',
  ]
else
  libadapta_private_sources += [
    'adap-settings-impl-portal.c',
  ]
endif

libadapta_sources = [
  libadapta_generated_headers,
  libadapta_public_sources,
  libadapta_private_sources,
  libadapta_resources,
  libadapta_stylesheet_resources,
  libadapta_init_public_types,
]

configure_file(
         output: 'config.h',
  configuration: config_h,
)

libadapta_link_args = []
libadapta_symbols_file = 'libadapta.syms'

# Check linker flags
ld_version_script_arg = '-Wl,--version-script,@0@/@1@'.format(meson.project_source_root(),
                                                              libadapta_symbols_file)
if cc.links('int main() { return 0; }', args : ld_version_script_arg, name : 'ld_supports_version_script')
  libadapta_link_args += [ld_version_script_arg]
endif

# set default libdir on win32 for libadapta target to keep MinGW compatibility
if target_system == 'windows'
  adapta_libdir = [true]
else
  adapta_libdir = libdir
endif

libadapta = library(
  'adapta-' + apiversion,
  libadapta_sources,

              soversion: soversion,
                 c_args: libadapta_c_args,
           dependencies: libadapta_deps,
    include_directories: [ root_inc, src_inc ],
                install: true,
              link_args: libadapta_link_args,
            install_dir: adapta_libdir,
  gnu_symbol_visibility: 'hidden',
)

libadapta_dep = declare_dependency(
              sources: libadapta_generated_headers,
         dependencies: libadapta_public_deps,
            link_with: libadapta,
  include_directories: include_directories('.'),
)

meson.override_dependency(package_api_name, libadapta_dep)

if introspection

   libadapta_gir_extra_args = [
     '--c-include=adapta.h',
     '--quiet',
     '-DADAPTA_COMPILATION',
   ]

   libadapta_gir = gnome.generate_gir(libadapta,
                 sources: libadapta_generated_headers + libadapta_public_headers + libadapta_public_sources,
               nsversion: apiversion,
               namespace: 'Adap',
         export_packages: package_api_name,
           symbol_prefix: 'adap',
       identifier_prefix: 'Adap',
               link_with: libadapta,
                includes: ['Gio-2.0', 'Gtk-4.0'],
                 install: true,
         install_dir_gir: girdir,
     install_dir_typelib: typelibdir,
              extra_args: libadapta_gir_extra_args,
   )

   if get_option('vapi')

     libadapta_vapi = gnome.generate_vapi(package_api_name,
             sources: libadapta_gir[0],
            packages: [ 'gio-2.0', 'gtk4' ],
             install: true,
         install_dir: vapidir,
       metadata_dirs: [ meson.current_source_dir() ],
     )

   endif
endif

pkgg = import('pkgconfig')

pkgg.generate(
    libraries: [libadapta],
      subdirs: libadapta_header_subdir,
      version: meson.project_version(),
         name: 'Adapta',
     filebase: package_api_name,
  description: 'Building blocks for modern GNOME applications',
     requires: 'gtk4',
  install_dir: libdir / 'pkgconfig',
)
