project(
  'clas12-analysis',
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

fmt_dep     = subproject('fmt').get_variable('fmt_dep')
toml_dep    = subproject('tomlplusplus').get_variable('tomlplusplus_dep')
lz4_dep     = subproject('lz4').get_variable('liblz4_dep')
eigen_dep   = dependency('eigen3')
cxxopts_dep = dependency('cxxopts')

#══════════════════════════════════════════════════════════════════════════════#
#                              EXTERNAL LIBRARIES                               #
#══════════════════════════════════════════════════════════════════════════════#

# HIPO4 library
subdir('extern/hipo4')

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
#                              CORE LIBRARIES                                   #
#══════════════════════════════════════════════════════════════════════════════#

# Framework - Core reusable analysis framework
subdir('framework')

# CLAS12 - CLAS12-specific components
subdir('clas12')

# Algorithms - Standard physics algorithms
subdir('algorithms')

#══════════════════════════════════════════════════════════════════════════════#
#                              EXECUTABLES                                      #
#══════════════════════════════════════════════════════════════════════════════#

# User analyses
subdir('analyses')

# Learning examples
subdir('examples')

# Utility tools
subdir('tools')

#══════════════════════════════════════════════════════════════════════════════#
#                              TESTS                                            #
#══════════════════════════════════════════════════════════════════════════════#

subdir('test')
