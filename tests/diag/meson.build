# Diag module tests and demos

# Include source headers
diag_inc = include_directories('../../src')

if get_option('build_demos')
  # Diag functional demo
  diag_demo = executable('diag_demo', 
    'diag_demo.cc',
    dependencies: [libdiag, magic_enum_dep],
    include_directories: diag_inc,
    install: false
  )
  
  # Custom target to run diag demo
  custom_target('run-diag-demo',
    command: [diag_demo],
    depends: [diag_demo],
    build_by_default: false,
    output: 'diag_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Diag unit tests
  diag_test = executable('diag_test',
    'diag_test.cc',
    dependencies: [libdiag, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: diag_inc,
    install: false
  )
  
  test('diag_unit_test', diag_test, suite: 'diag')
endif
