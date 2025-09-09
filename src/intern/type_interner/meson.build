inc_dir = include_directories('.')
type_interner_sources = ['type_interner.cc']
libtype_interner_sta = static_library('type_interner', type_interner_sources, include_directories: inc_dir)
libtype_interner = declare_dependency(link_with: libtype_interner_sta, include_directories: inc_dir)
