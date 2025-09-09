inc_dir = include_directories('.', '..')
ast_sources = ['ast.cc']
libast_sta = static_library('ast', ast_sources, include_directories: inc_dir)
libast = declare_dependency(link_with: libast_sta, include_directories: inc_dir)