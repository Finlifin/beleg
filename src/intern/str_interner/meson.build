inc_dir = include_directories('.')
str_interner_sources = ['str_interner.cc']
libstr_interner_sta = static_library('str_interner', str_interner_sources, include_directories: inc_dir)
libstr_interner = declare_dependency(link_with: libstr_interner_sta, include_directories: inc_dir)
