# Lex module tests and demos

# Include source headers
lex_inc = include_directories('../../src')

if get_option('build_demos')
  # Lex functional demo
  lex_demo = executable('lex_demo', 
    'lex_demo.cc',
    dependencies: [liblex, magic_enum_dep],
    include_directories: lex_inc,
    install: false
  )
  
  # Custom target to run lex demo
  custom_target('run-lex-demo',
    command: [lex_demo],
    depends: [lex_demo],
    build_by_default: false,
    output: 'lex_demo_output.txt'
  )
endif

if get_option('build_tests')
  # Lex unit tests
  lex_test = executable('lex_test',
    'lex_test.cc',
    dependencies: [liblex, gtest_dep, gtest_main_dep, magic_enum_dep],
    include_directories: lex_inc,
    install: false
  )
  
  test('lex_unit_test', lex_test, suite: 'lex')
endif
