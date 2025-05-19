if get_option('gtk_doc')

subdir('tools')

expand_content_md_files = [
  'adaptive-layouts.md',
  'boxed-lists.md',
  'build-howto.md',
  'initialization.md',
  'migrating-libhandy-1-4-to-libadapta.md',
  'migrating-to-adaptive-dialogs.md',
  'migrating-to-breakpoints.md',
  'named-colors.md',
  'styles-and-appearance.md',
  'style-classes.md',
  'widget-gallery.md',
]

toml_data = configuration_data()
toml_data.set('VERSION', meson.project_version())

libadapta_toml = configure_file(
  input: 'libadapta.toml.in',
  output: 'libadapta.toml',
  configuration: toml_data
)

dependency('gi-docgen', version: '>= 2021.1',
           fallback: ['gi-docgen', 'dummy_dep'],
           native: true,
           required: get_option('gtk_doc'))

gidocgen = find_program('gi-docgen')

docs_dir = datadir / 'doc'

custom_target('libadapta-doc',
  input: [ libadapta_toml, libadapta_gir[0] ],
  output: 'libadapta-@0@'.format(apiversion),
  command: [
    gidocgen,
    'generate',
    '--quiet',
    '--config=@INPUT0@',
    '--output-dir=@OUTPUT@',
    '--no-namespace-dir',
    '--content-dir=@0@'.format(meson.current_source_dir()),
    '@INPUT1@',
  ],
  depend_files: [ expand_content_md_files ],
  build_by_default: true,
  install: true,
  install_dir: docs_dir,
)

endif
