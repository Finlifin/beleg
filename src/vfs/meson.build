vfs_sources = files(
  'vfs.cc',
)

vfs_lib = static_library(
  'vfs',
  vfs_sources,
  dependencies: [
    libsource_map,
    libast,
  ],
  include_directories: [
    include_directories('..'),
  ],
)

vfs_dep = declare_dependency(
  link_with: vfs_lib,
  include_directories: [
    include_directories('.'),
  ],
  dependencies: [
    libsource_map,
    libast,
  ],
)
