# Algorithms library - Standard physics algorithms
# Provides selection, reconstruction, analysis, and monitoring algorithms

algorithms_inc = include_directories('include')

algorithms_sources = files(
  'src/Pi0ReconstructionAlg.cpp',
)

# Build algorithms as shared library
algorithms_lib = library(
  'clas12-algorithms',
  algorithms_sources,
  include_directories: algorithms_inc,
  dependencies: [framework_dep, clas12_dep, root_dep],
  install: true,
)

# Dependency object for user analyses
algorithms_dep = declare_dependency(
  include_directories: algorithms_inc,
  link_with: algorithms_lib,
  dependencies: [framework_dep, clas12_dep],
)

# Install headers
install_subdir('include/clas12', install_dir: get_option('includedir'))
