# Intern module tests and demos

# Include source headers
intern_inc = include_directories('../../src')

if get_option('build_demos')
  # Intern functional demo
  intern_demo = executable('intern_demo', 
    'intern_demo.cc',
    dependencies: [libintern, magic_enum_dep],
    include_directories: intern_inc,
    install: false
  )
  
  # Custom target to run intern demo
  custom_target('run-intern-demo',
    command: [intern_demo],
    depends: [intern_demo],
    build_by_default: false,
    output: 'intern_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Intern unit tests
  intern_test = executable('intern_test',
    'intern_test.cc',
    dependencies: [libintern, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: intern_inc,
    install: false
  )
  
  test('intern_unit_test', intern_test, suite: 'intern')
endif
