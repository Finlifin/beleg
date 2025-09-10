# Parse module tests and demos

# Include source headers
parse_inc = include_directories('../../src')

if get_option('build_demos')
  # Parse functional demo
  parse_demo = executable('parse_demo', 
    'parse_demo.cc',
    dependencies: [libparse, magic_enum_dep],
    include_directories: parse_inc,
    install: false
  )
  
  # Custom target to run parse demo
  custom_target('run-parse-demo',
    command: [parse_demo],
    depends: [parse_demo],
    build_by_default: false,
    output: 'parse_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Parse unit tests
  parse_test = executable('parse_test',
    'parse_test.cc',
    dependencies: [libparse, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: parse_inc,
    install: false
  )
  
  test('parse_unit_test', parse_test, suite: 'parse')
endif
