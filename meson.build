project(
  'beleg',
  'cpp',
  version : '0.1',
  meson_version : '>= 1.3.0',
  default_options : ['warning_level=3', 'cpp_std=c++23'],
)

magic_enum_proj = subproject('magic_enum')
magic_enum_dep = magic_enum_proj.get_variable('magic_enum_dep')

dependencies = [magic_enum_dep]

subdir('src')
subdir('tests')

test('basic', beleg_exe)
