project(
  'beleg',
  'cpp',
  version : '0.1',
  meson_version : '>= 1.3.0',
  default_options : ['warning_level=3', 'cpp_std=c++20'],
)

dependencies = [
]

subdir('src')
subdir('tests')

test('basic', beleg_exe)
