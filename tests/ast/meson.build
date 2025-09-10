# AST module tests and demos

# Include source headers
ast_inc = include_directories('../../src')

if get_option('build_demos')
  # AST functional demo
  ast_demo = executable('ast_demo', 
    'ast_demo.cc',
    dependencies: [libast, magic_enum_dep],
    include_directories: ast_inc,
    install: false
  )
  
  # Custom target to run ast demo
  custom_target('run-ast-demo',
    command: [ast_demo],
    depends: [ast_demo],
    build_by_default: false,
    output: 'ast_demo_output.txt'
  )
endif

if get_option('build_tests')
  # AST unit tests
  ast_test = executable('ast_test',
    'ast_test.cc',
    dependencies: [libast, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: ast_inc,
    install: false
  )
  
  test('ast_unit_test', ast_test, suite: 'ast')
endif
