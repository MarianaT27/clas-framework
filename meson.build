project(
  'clas12-framework',
  'cpp',
  version: '2.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'buildtype=release',
    'strip=true',
  ],
)

#── Subprojects ───────────────────────────────────────────────────────────────#
fmt = subproject('fmt')
fmt_dep = fmt.get_variable('fmt_dep')

toml = subproject('tomlplusplus')
toml_dep = toml.get_variable('tomlplusplus_dep')

lz4 = subproject('lz4')
lz4_dep = lz4.get_variable('liblz4_dep')

subdir('hipo4')

#── ROOT via root-config ─────────────────────────────────────────────────────#
root_incdir   = run_command('root-config', '--incdir', check: true).stdout().strip()
root_libs     = run_command('root-config', '--libs',   check: true).stdout().strip().split()

root_dep = declare_dependency(
  include_directories: include_directories(root_incdir),
  link_args:           root_libs,
)

#── Project sources & include dirs ────────────────────────────────────────────#

inc = include_directories('include')

# Original study1 executable (legacy) - COMMENTED OUT - source files deleted
# sources = files(
#   'src/Core/Particle.cpp',
#   'src/Framework/Histogram.cpp',
#   'src/main.cpp',
#   'src/study1/Drawing.cpp',
#   'src/study1/Reader.cpp',
# )
#
# executable(
#   'starter',
#   sources,
#   include_directories: [inc],
#   dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
# )

# New framework example executable
framework_example_sources = files(
  'src/Core/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/example_framework.cpp',
)

executable(
  'framework_example',
  framework_example_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)

# Improved main with Phase 3 features
main_improved_sources = files(
  'src/Core/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/main_improved.cpp',
)

executable(
  'main_improved',
  main_improved_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)

# Advanced main with algorithm types demo
main_advanced_sources = files(
  'src/Core/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/main_advanced.cpp',
)

executable(
  'main_advanced',
  main_advanced_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)

# Automatic histogram writing demo
auto_histograms_sources = files(
  'src/Core/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/example_auto_histograms.cpp',
)

executable(
  'example_auto_histograms',
  auto_histograms_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)

# Pi0 reconstruction analysis executable
main_pi0_sources = files(
  'src/Core/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/Algorithms/Pi0ReconstructionAlg.cpp',
  'src/main_pi0.cpp',
)

executable(
  'main_pi0',
  main_pi0_sources,
  include_directories: [inc],
  dependencies: [hipo_dep, fmt_dep, toml_dep, root_dep],
)

#── Test suite ────────────────────────────────────────────────────────────────#
# Tests require Google Test and RapidCheck for property-based testing
# subdir('test')

#── Python bindings (optional) ────────────────────────────────────────────────#
# Uncomment to build Python bindings (requires Python 3 and pybind11)
# subdir('python')