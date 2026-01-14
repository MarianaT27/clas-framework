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
#                              COMPILER REQUIREMENTS                            #
#══════════════════════════════════════════════════════════════════════════════#

# C++20/23 features require modern compilers
# - GCC 12+ for full C++20 coroutine support
# - Clang 14+ for full C++20 coroutine support

cpp = meson.get_compiler('cpp')
compiler_id = cpp.get_id()
compiler_version = cpp.version()

# Verify compiler compatibility
if compiler_id == 'gcc'
  if not cpp.version().version_compare('>=12')
    error('GCC 12 or later is required for C++20 coroutine support. Found: ' + compiler_version)
  endif
elif compiler_id == 'clang'
  if not cpp.version().version_compare('>=14')
    error('Clang 14 or later is required for C++20 coroutine support. Found: ' + compiler_version)
  endif
elif compiler_id == 'msvc'
  if not cpp.version().version_compare('>=19.29')
    error('MSVC 19.29 (VS 2019 16.10) or later is required for C++20 support. Found: ' + compiler_version)
  endif
else
  warning('Unknown compiler: ' + compiler_id + '. C++20 coroutine support may be incomplete.')
endif

# Compiler flags for C++20 coroutine support
cpp20_args = []

if compiler_id == 'gcc'
  # GCC requires -fcoroutines for coroutine support (GCC 10-12)
  # GCC 13+ has coroutines enabled by default with -std=c++20
  if cpp.version().version_compare('<13')
    cpp20_args += ['-fcoroutines']
  endif
elif compiler_id == 'clang'
  # Clang 14+ supports coroutines natively with -std=c++20
  # No additional flags needed
endif

# Add global C++20 arguments
add_project_arguments(cpp20_args, language: 'cpp')

message('Compiler: ' + compiler_id + ' ' + compiler_version)
message('C++20 coroutine support: enabled')

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
  'advanced':               {'main': 'src/advanced.cpp'},
  'example_auto_histograms': {'main': 'src/example_auto_histograms.cpp'},
  'example_builder':        {'main': 'src/example_builder.cpp'},
  'example_simple_algorithm': {'main': 'src/example_simple_algorithm.cpp'},
  'example_modern_electron_analysis': {'main': 'src/example_modern_electron_analysis.cpp'},
  'lambda':                 {'main': 'src/lambda.cpp'},
  'decays_lambda':          {'main': 'src/decays_lambda.cpp'},
  'electron_analysis':      {'main': 'src/electron_analysis.cpp'},
  'pion_analysis':          {'main': 'src/pion_analysis.cpp'},
  'nuclear_transparency':   {'main': 'src/nuclear_transparency.cpp'},
  'dipion_filter':         {'main': 'src/dipion_filter.cpp'},
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
