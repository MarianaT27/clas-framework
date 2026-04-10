# CLAS12 library - pure data-type layer (Particle, PIDs, Target, physics
# helpers). Depends only on external libraries; never on framework.
# Framework depends on clas12_dep, not the reverse.

clas12_inc = include_directories('include')

clas12_sources = files(
  'src/Particle.cpp',
)

clas12_lib = library(
  'clas12',
  clas12_sources,
  include_directories: clas12_inc,
  dependencies: [fmt_dep, hipo4_dep, root_dep, eigen_dep],
  install: true,
)

clas12_dep = declare_dependency(
  include_directories: clas12_inc,
  link_with: clas12_lib,
  dependencies: [fmt_dep, hipo4_dep, root_dep, eigen_dep],
)

install_subdir('include/clas12', install_dir: get_option('includedir'))
