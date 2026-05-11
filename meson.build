project(
  'clas12-analysis',
  'cpp',
  version: '2.0',
  default_options: [
    'cpp_std=c++23',
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

# Optional strict-warning profile, opt in via -Dstrict_warnings=true.
# CI turns this on; local dev keeps the lax default.
if get_option('strict_warnings')
  strict_args = [
    '-Wextra',
    '-Wconversion',
    '-Wshadow',
    '-Wnon-virtual-dtor',
    '-Wold-style-cast',
    '-Wcast-align',
    '-Wpedantic',
    '-Werror',
  ]
  # Filter to flags the compiler actually accepts.
  strict_args = cpp.get_supported_arguments(strict_args)
  add_project_arguments(strict_args, language: 'cpp')
  message('Strict warnings enabled: ' + ' '.join(strict_args))
endif

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

zlib_dep    = dependency('zlib')
thread_dep  = dependency('threads')
project_libs = []

# mh — header-only histogram library at extern/mh/.
# MH_NO_MDSPAN forces mh::StaticStorage off mh's optional std::mdspan view.
# Some C++23 stdlibs ship <mdspan> without putting std::mdspan/std::dextents
# in std, which trips mh's __has_include gate. We don't use StaticStorage
# anywhere, so disable the path unconditionally.
#
# `mh::AtomicStorage<WeightedAccumulator>` does 128-bit (16-byte) atomic
# loads/CAS on the {sum_w, sum_w2} pair. On Linux/GCC those dispatch to
# libatomic intrinsics (__atomic_load_16, __atomic_compare_exchange_16);
# Apple libc++ inlines them so macOS builds don't need the explicit link.
# `find_library('atomic', required: false)` returns a no-op dependency on
# macOS, so threading it through mh_dep is a one-liner that works on both
# platforms.
atomic_dep = cpp.find_library('atomic', required: false)
mh_dep = declare_dependency(
  include_directories: include_directories('extern'),
  compile_args: ['-DMH_NO_MDSPAN'],
  dependencies: [atomic_dep],
)

#══════════════════════════════════════════════════════════════════════════════#
#                              ROOT DEPENDENCY                                  #
#══════════════════════════════════════════════════════════════════════════════#

root_dep = declare_dependency(
  include_directories: include_directories(
    run_command('root-config', '--incdir', check: true).stdout().strip()
  ),
  link_args: run_command('root-config', '--libs', check: true).stdout().strip().split(),
)

# Alias for the casing used by extern/hipo4/meson.build.
ROOT_dep = root_dep

# HIPO4 library — needs root_dep above for the optional datastreamROOT path
# (root:// / http:// / https:// / davix:// over TFile::Open).
subdir('extern/hipo4')

# Alias for downstream meson.build files that still reference the legacy
# `hipo4_dep` name (extern/hipo4/meson.build declares it as `hipo_dep`).
hipo4_dep = hipo_dep

#══════════════════════════════════════════════════════════════════════════════#
#                              CORE LIBRARIES                                   #
#══════════════════════════════════════════════════════════════════════════════#

# CLAS12 - pure data types (Particle, PIDs, Target, physics helpers).
# Has zero dependency on framework, so it builds first.
subdir('clas12')

# Framework - algorithm/service infrastructure. Consumes clas12 types,
# so it depends on clas12_dep (dependency inverted from the previous
# setup which hid the real direction behind an include-path hack).
subdir('framework')

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
