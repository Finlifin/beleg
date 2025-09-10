# Source_map module tests and demos

# Include source headers
source_map_inc = include_directories('../../src')

if get_option('build_demos')
  # Source_map functional demo
  source_map_demo = executable('source_map_demo', 
    'source_map_demo.cc',
    dependencies: [libsource_map, magic_enum_dep],
    include_directories: source_map_inc,
    install: false
  )
  
  # Custom target to run source_map demo
  custom_target('run-source_map-demo',
    command: [source_map_demo],
    depends: [source_map_demo],
    build_by_default: false,
    output: 'source_map_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Source_map unit tests
  source_map_test = executable('source_map_test',
    'source_map_test.cc',
    dependencies: [libsource_map, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: source_map_inc,
    install: false
  )
  
  test('source_map_unit_test', source_map_test, suite: 'source_map')
  
  # Load file feature test
  load_file_test = executable('load_file_test',
    'load_file_test.cc',
    dependencies: [libsource_map, magic_enum_dep],
    include_directories: source_map_inc,
    install: false
  )
endif
