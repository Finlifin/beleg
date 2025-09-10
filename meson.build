project(
  'beleg',
  'cpp',
  version : '0.1',
  meson_version : '>= 1.3.0',
  default_options : ['warning_level=3', 'cpp_std=c++23'],
)

magic_enum_proj = subproject('magic_enum')
magic_enum_dep = magic_enum_proj.get_variable('magic_enum_dep')

# Get gtest dependency
gtest_proj = subproject('gtest', required : get_option('build_tests'))
if get_option('build_tests')
  gtest_dep = gtest_proj.get_variable('gtest_dep')
  gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')
endif

dependencies = [magic_enum_dep]

subdir('src')

if get_option('build_tests') or get_option('build_demos')
  subdir('tests')
endif
