# CLAS12 library - pure data-type layer (Particle, PIDs, Target, physics
# helpers). Depends on framework headers for Event so Particle.cpp can
# implement its detector accessors via the generated read_*_bank helpers.
# It does NOT link against framework — only the headers cross.

clas12_inc = include_directories('include')
# Framework headers are referenced by Particle.cpp through ReadBank.hpp.
# We include the path by hand here so we don't need framework_dep in the
# clas12 library (which would invert the link-layer dependency).
framework_inc_for_clas12 = include_directories('../framework/include')

clas12_sources = files(
  'src/Particle.cpp',
)

clas12_lib = library(
  'clas12',
  clas12_sources,
  include_directories: [clas12_inc, framework_inc_for_clas12],
  dependencies: [fmt_dep, hipo4_dep, root_dep, eigen_dep],
  install: true,
)

clas12_dep = declare_dependency(
  include_directories: clas12_inc,
  link_with: clas12_lib,
  dependencies: [fmt_dep, hipo4_dep, root_dep, eigen_dep],
)

install_subdir('include/clas12', install_dir: get_option('includedir'))
