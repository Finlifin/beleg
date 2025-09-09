inc_dir = include_directories('.', '..')
hir_sources = ['hir.cc']
libhir_sta = static_library('hir', hir_sources, include_directories: inc_dir)
libhir = declare_dependency(link_with: libhir_sta, include_directories: inc_dir)
