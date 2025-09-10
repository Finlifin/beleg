# Codegen module tests and demos

# Include source headers
codegen_inc = include_directories('../../src')

if get_option('build_demos')
  # Codegen functional demo
  codegen_demo = executable('codegen_demo', 
    'codegen_demo.cc',
    dependencies: [libcodegen, magic_enum_dep],
    include_directories: codegen_inc,
    install: false
  )
  
  # Custom target to run codegen demo
  custom_target('run-codegen-demo',
    command: [codegen_demo],
    depends: [codegen_demo],
    build_by_default: false,
    output: 'codegen_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Codegen unit tests
  codegen_test = executable('codegen_test',
    'codegen_test.cc',
    dependencies: [libcodegen, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: codegen_inc,
    install: false
  )
  
  test('codegen_unit_test', codegen_test, suite: 'codegen')
endif
