inc_dir = include_directories('.', '..')
source_map_sources = ['source_map.cc']
libsource_map_sta = static_library('source_map', source_map_sources, include_directories: inc_dir)
libsource_map = declare_dependency(link_with: libsource_map_sta, include_directories: inc_dir)
