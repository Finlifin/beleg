vfs_test = executable('vfs_test',
  'vfs_test.cc',
  dependencies: [
    gtest_dep,
    gtest_main_dep,
    vfs_dep,
  ],
  include_directories: [
    include_directories('../..'),
  ],
)

test('vfs_unit_test', vfs_test, suite: 'beleg:vfs')
