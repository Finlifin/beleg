inc_dir = include_directories('.', '..')
parse_sources = ['parse.cc']
libparse_sta = static_library('parse', parse_sources, include_directories: inc_dir)
libparse = declare_dependency(link_with: libparse_sta, include_directories: inc_dir)
