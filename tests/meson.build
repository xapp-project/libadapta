if get_option('tests')

subdir('manual')

test_resources = gnome.compile_resources(
   'adapta-test-resources',
   'tests.gresources.xml',

   c_name: 'test',
)

test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly',
  'GSETTINGS_BACKEND=memory',
  'PYTHONDONTWRITEBYTECODE=yes',
  'MALLOC_CHECK_=2',
  'GTK_A11Y=none',
]

test_cflags = [
  '-DADAP_LOG_DOMAIN="Adapta"',
]

test_link_args = []
use_pie = false

if cc.get_argument_syntax() != 'msvc'
  test_link_args += '-fPIC'
  use_pie = true
endif

test_names = [
  'test-about-dialog',
  'test-about-window',
  'test-action-row',
  'test-alert-dialog',
  'test-animation',
  'test-animation-target',
  'test-application-window',
  'test-avatar',
  'test-banner',
  'test-bin',
  'test-breakpoint',
  'test-breakpoint-bin',
  'test-button-content',
  'test-carousel',
  'test-carousel-indicator-dots',
  'test-carousel-indicator-lines',
  'test-combo-row',
  'test-dialog',
  'test-easing',
  'test-entry-row',
  'test-expander-row',
  'test-flap',
  'test-header-bar',
  'test-leaflet',
  'test-message-dialog',
  'test-navigation-split-view',
  'test-navigation-view',
  'test-overlay-split-view',
  'test-password-entry-row',
  'test-preferences-dialog',
  'test-preferences-group',
  'test-preferences-page',
  'test-preferences-row',
  'test-preferences-window',
  'test-spin-row',
  'test-split-button',
  'test-squeezer',
  'test-status-page',
  'test-style-manager',
  'test-switch-row',
  'test-tab-bar',
  'test-tab-button',
  'test-tab-overview',
  'test-tab-view',
  'test-timed-animation',
  'test-toast',
  'test-toast-overlay',
  'test-toolbar-view',
  'test-view-switcher',
  'test-view-switcher-bar',
  'test-window',
  'test-window-title',
]

foreach test_name : test_names
  test_sources = [
    test_name + '.c',
    test_resources,
    libadapta_generated_headers
  ]

  t = executable(test_name, test_sources,
                       c_args: test_cflags,
                    link_args: test_link_args,
                 dependencies: libadapta_deps + [libadapta_dep],
                          pie: use_pie,
                )
  test(test_name, t, env: test_env)
endforeach

endif
