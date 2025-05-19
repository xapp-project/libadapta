test_cflags = [
  '-DADAP_LOG_DOMAIN="Adapta"',
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

test_names = [
  'test-alert-dialogs',
  'test-avatar-colors',
  'test-breakpoints',
  'test-button-states',
  'test-navigation',
  'test-split-views',
  'test-toolbars',
  'test-view-switcher-bars',
]

foreach test_name : test_names
  executable(test_name,
             [test_name + '.c'] + libadapta_generated_headers,
             c_args: test_cflags,
             dependencies: libadapta_deps + [libadapta_dep])
endforeach
