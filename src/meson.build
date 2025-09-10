subdir('source_map')
subdir('ast')
subdir('codegen')
subdir('diag')
subdir('driver')
subdir('hir')
subdir('intern')
subdir('lex')
subdir('parse')

inc = [include_directories('.')]

libbeleg = declare_dependency(dependencies: [
    libast,
    libcodegen,
    libdiag,
    libdriver,
    libhir,
    libintern,
    liblex,
    libparse,
    libsource_map
])

beleg_exe = executable('beleg', 'main.cc',
    dependencies: libbeleg,
    include_directories: inc,
)