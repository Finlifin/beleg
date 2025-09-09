inc_dir = include_directories('.', '..')
codegen_sources = ['codegen.cc']
libcodegen_sta = static_library('codegen', codegen_sources, include_directories: inc_dir)
libcodegen = declare_dependency(link_with: libcodegen_sta, include_directories: inc_dir)
