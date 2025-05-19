if get_option('examples')

subdir('data')

demo_config_data = configuration_data()
demo_config_data.set_quoted('ADAP_METAINFO', appstream_file.full_path())
demo_config_data.set_quoted('ADAP_DEMO_VCS_TAG', '@VCS_TAG@')

demo_config_h = vcs_tag(
  input: configure_file(
    output: 'config.h.in',
    configuration: demo_config_data
  ),
  output: 'config.h'
)

adapta_demo_resources = gnome.compile_resources(
   'adapta-demo-resources',
   'adapta-demo.gresources.xml',

   c_name: 'adap',
   dependencies: appstream_file,
   source_dir: meson.current_build_dir(),
)

adapta_demo_sources = [
  adapta_demo_resources,

  'pages/about/adap-demo-page-about.c',
  'pages/animations/adap-demo-page-animations.c',
  'pages/avatar/adap-demo-page-avatar.c',
  'pages/banners/adap-demo-page-banners.c',
  'pages/buttons/adap-demo-page-buttons.c',
  'pages/carousel/adap-demo-page-carousel.c',
  'pages/clamp/adap-demo-page-clamp.c',
  'pages/dialogs/adap-demo-page-dialogs.c',
  'pages/lists/adap-demo-page-lists.c',
  'pages/navigation-view/adap-demo-page-navigation-view.c',
  'pages/navigation-view/adap-navigation-view-demo-window.c',
  'pages/split-views/adap-demo-page-split-views.c',
  'pages/split-views/adap-navigation-split-view-demo-window.c',
  'pages/split-views/adap-overlay-split-view-demo-window.c',
  'pages/styles/adap-demo-page-styles.c',
  'pages/styles/adap-style-demo-window.c',
  'pages/tab-view/adap-demo-page-tab-view.c',
  'pages/tab-view/adap-tab-view-demo-page.c',
  'pages/tab-view/adap-tab-view-demo-window.c',
  'pages/toasts/adap-demo-page-toasts.c',
  'pages/view-switcher/adap-demo-page-view-switcher.c',
  'pages/view-switcher/adap-view-switcher-demo-window.c',
  'pages/welcome/adap-demo-page-welcome.c',

  'adapta-demo.c',
  'adap-demo-debug-info.c',
  'adap-demo-preferences-window.c',
  'adap-demo-window.c',
  demo_config_h,
  libadapta_generated_headers,
]

adapta_demo = executable('adapta-@0@-demo'.format(apiversion),
  adapta_demo_sources,
  dependencies: libadapta_dep,
  install: true,
)

endif
