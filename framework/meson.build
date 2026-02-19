# Framework library - Core reusable analysis framework
# Provides algorithms, services, event model, and utilities

framework_inc = include_directories('include')
# Framework needs clas12 headers for Event.hpp - note this creates tight coupling
clas12_header_inc = include_directories('../clas12/include')

framework_sources = files(
  'src/Histogram.cpp',
  'src/services/PhotonCutsService.cpp',
)

# Build framework as shared library
framework_lib = library(
  'clas12-framework',
  framework_sources,
  include_directories: [framework_inc, clas12_header_inc],
  dependencies: [fmt_dep, toml_dep, root_dep, hipo4_dep, eigen_dep],
  install: true,
)

# Dependency object for downstream libraries
framework_dep = declare_dependency(
  include_directories: [framework_inc, clas12_header_inc],
  link_with: framework_lib,
  dependencies: [fmt_dep, toml_dep, hipo4_dep, eigen_dep],
)

# Install headers
install_subdir('include/clas12', install_dir: get_option('includedir'))
