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

#══════════════════════════════════════════════════════════════════════════════#
#                              SUBPROJECTS                                      #
#══════════════════════════════════════════════════════════════════════════════#

fmt_dep    = subproject('fmt').get_variable('fmt_dep')
toml_dep   = subproject('tomlplusplus').get_variable('tomlplusplus_dep')
lz4_dep    = subproject('lz4').get_variable('liblz4_dep')
cxxopts_dep = dependency('cxxopts')

subdir('hipo4')

#══════════════════════════════════════════════════════════════════════════════#
#                              ROOT DEPENDENCY                                  #
#══════════════════════════════════════════════════════════════════════════════#

root_dep = declare_dependency(
  include_directories: include_directories(
    run_command('root-config', '--incdir', check: true).stdout().strip()
  ),
  link_args: run_command('root-config', '--libs', check: true).stdout().strip().split(),
)

#══════════════════════════════════════════════════════════════════════════════#
#                              COMMON SOURCES                                   #
#══════════════════════════════════════════════════════════════════════════════#

inc = include_directories('include')

# Shared framework sources used by most executables
framework_core = files(
  'src/CLAS/Particle.cpp',
  'src/Framework/Histogram.cpp',
  'src/Algorithms/Pi0ReconstructionAlg.cpp',
  'src/Framework/Services/PhotonCutsService.cpp',
)

# Common dependencies for all executables
common_deps = [hipo_dep, fmt_dep, root_dep]
full_deps   = common_deps + [toml_dep, cxxopts_dep]

#══════════════════════════════════════════════════════════════════════════════#
#                              EXECUTABLES                                      #
#══════════════════════════════════════════════════════════════════════════════#

# Helper to build standard executables with framework core
executables = {
  'framework_example':      {'main': 'src/example_framework.cpp'},
  'improved':               {'main': 'src/improved.cpp'},
  'advanced':               {'main': 'src/advanced.cpp'},
  'example_auto_histograms': {'main': 'src/example_auto_histograms.cpp'},
  'lambda':                 {'main': 'src/lambda.cpp'},
  'decays_lambda':          {'main': 'src/decays_lambda.cpp'},
  'electron_analysis':      {'main': 'src/electron_analysis.cpp'},
  'pion_analysis':          {'main': 'src/pion_analysis.cpp'},
}

foreach name, config : executables
  executable(
    name,
    framework_core + files(config['main']),
    include_directories: inc,
    dependencies: full_deps,
  )
endforeach

# Pi0 - requires additional algorithm sources
executable(
  'pi0',
  framework_core + files(
    'src/Framework/Services/PhotonCutsService.cpp',
    'src/Algorithms/Pi0ReconstructionAlg.cpp',
    'src/pi0.cpp',
  ),
  include_directories: inc,
  dependencies: full_deps,
)

# Generic batch job submitter - works with any analysis executable
executable(
  'batch',
  files('src/batch.cpp'),
  include_directories: inc,
  dependencies: common_deps + [cxxopts_dep],
)

#══════════════════════════════════════════════════════════════════════════════#
#                              OPTIONAL COMPONENTS                              #
#══════════════════════════════════════════════════════════════════════════════#

# Test suite (requires Google Test)
# subdir('test')

# Python bindings (requires pybind11)
# subdir('python')
