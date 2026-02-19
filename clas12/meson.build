# CLAS12 library - CLAS12-specific components
# Provides particle definitions, bank readers, and physics utilities

clas12_inc = include_directories('include')

clas12_sources = files(
  'src/Particle.cpp',
)

# Build CLAS12 as shared library
clas12_lib = library(
  'clas12',
  clas12_sources,
  include_directories: clas12_inc,
  dependencies: [framework_dep, hipo4_dep, root_dep],
  install: true,
)

# Dependency object for downstream libraries
clas12_dep = declare_dependency(
  include_directories: clas12_inc,
  link_with: clas12_lib,
  dependencies: [framework_dep, hipo4_dep],
)

# Install headers
install_subdir('include/clas12', install_dir: get_option('includedir'))
