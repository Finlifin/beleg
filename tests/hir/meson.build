# Hir module tests and demos

# Include source headers
hir_inc = include_directories('../../src')

if get_option('build_demos')
  # Hir functional demo
  hir_demo = executable('hir_demo', 
    'hir_demo.cc',
    dependencies: [libhir, magic_enum_dep],
    include_directories: hir_inc,
    install: false
  )
  
  # Custom target to run hir demo
  custom_target('run-hir-demo',
    command: [hir_demo],
    depends: [hir_demo],
    build_by_default: false,
    output: 'hir_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Hir unit tests
  hir_test = executable('hir_test',
    'hir_test.cc',
    dependencies: [libhir, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: hir_inc,
    install: false
  )
  
  test('hir_unit_test', hir_test, suite: 'hir')
endif
