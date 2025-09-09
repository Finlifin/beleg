inc_dir = include_directories('.', '..')
driver_sources = ['driver.cc']
libdriver_sta = static_library('driver', driver_sources, include_directories: inc_dir)
libdriver = declare_dependency(link_with: libdriver_sta, include_directories: inc_dir)
