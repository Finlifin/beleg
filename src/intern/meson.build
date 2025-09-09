subdir('str_interner')
subdir('type_interner')

inc_dir = include_directories('.', '..')
intern_sources = []
libintern_sta = static_library('intern', intern_sources, include_directories: inc_dir)
libintern = declare_dependency(link_with: libintern_sta, include_directories: inc_dir, dependencies: [libstr_interner, libtype_interner])
