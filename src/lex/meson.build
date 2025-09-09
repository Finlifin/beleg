inc_dir = include_directories('.', '..')
deps = [magic_enum_dep]
lex_sources = ['lex.cc']
liblex_sta = static_library('lex', lex_sources, include_directories: inc_dir, dependencies: deps)
liblex = declare_dependency(link_with: liblex_sta, include_directories: inc_dir, dependencies: deps)
