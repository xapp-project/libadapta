screenshot_resources = gnome.compile_resources(
   'screenshot-resources',
   'screenshot.gresources.xml',

   c_name: 'adap',
)

screenshot_sources = [
  screenshot_resources,
  'screenshot.c',
  libadapta_generated_headers,
]

screenshot = executable('screenshot'.format(apiversion),
  screenshot_sources,
  dependencies: libadapta_deps + [libadapta_dep],
)
