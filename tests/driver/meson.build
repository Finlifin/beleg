# Driver module tests and demos

# Include source headers
driver_inc = include_directories('../../src')

if get_option('build_demos')
  # Driver functional demo
  driver_demo = executable('driver_demo', 
    'driver_demo.cc',
    dependencies: [libdriver, magic_enum_dep],
    include_directories: driver_inc,
    install: false
  )
  
  # Custom target to run driver demo
  custom_target('run-driver-demo',
    command: [driver_demo],
    depends: [driver_demo],
    build_by_default: false,
    output: 'driver_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Driver unit tests
  driver_test = executable('driver_test',
    'driver_test.cc',
    dependencies: [libdriver, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: driver_inc,
    install: false
  )
  
  test('driver_unit_test', driver_test, suite: 'driver')
endif
