inc_dir = include_directories('.', '..')
diag_sources = ['diag.cc', 'terminal_emitter.cc']
libdiag_sta = static_library('diag', diag_sources, 
  include_directories: inc_dir,
  dependencies: [libsource_map]
)
libdiag = declare_dependency(link_with: libdiag_sta, 
  include_directories: inc_dir,
  dependencies: [libsource_map]
)
