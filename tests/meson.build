# Tests meson.build

# Module list
modules = ['ast', 'codegen', 'diag', 'driver', 'hir', 'intern', 'lex', 'parse', 'source_map']

# Get options
test_module = get_option('test_module')
demo_module = get_option('demo_module')
build_tests = get_option('build_tests')
build_demos = get_option('build_demos')

# Build specific module or all modules
if test_module != '' and test_module in modules
  # Build only specified test module
  if build_tests
    subdir(test_module)
  endif
elif demo_module != '' and demo_module in modules
  # Build only specified demo module
  if build_demos
    subdir(demo_module)
  endif
else
  # Build all modules
  foreach module : modules
    subdir(module)
  endforeach
endif
