# CLAS12 library - pure header-only data-type layer (Particle, PIDs,
# Target, physics helpers, generated bank readers, and CLAS12-specific
# services). No .cpp sources, so no library object is produced — callers
# pull everything in via `clas12_dep`'s include path.

clas12_inc = include_directories('include')

clas12_dep = declare_dependency(
  include_directories: clas12_inc,
  dependencies: [fmt_dep, hipo4_dep, root_dep, eigen_dep],
)

install_subdir('include/clas12', install_dir: get_option('includedir'))
