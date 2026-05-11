hipo_sources = files(
  'bank.cpp',
  'datastream.cpp',
  'dictionary.cpp',
  'event.cpp',
  'fusion.cpp',
  'node.cpp',
  'parser.cpp',
  'reaction.cpp',
  'reader.cpp',
  'record.cpp',
  'recordbuilder.cpp',
  'tuple.cpp',
  'utils.cpp',
  'wrapper.cpp',
  'writer.cpp',
)
hipo_headers = files(
  'bank.h',
  'constants.h',
  'datastream.h',
  'dictionary.h',
  'event.h',
  'fizika.h',
  'filehandle.h',
  'fusion.h',
  'hipoexceptions.h',
  'ioOptions.h',
  'json.h',
  'jsonutil.h',
  'node.h',
  'parser.h',
  'reaction.h',
  'reader.h',
  'record.h',
  'recordbuilder.h',
  'recordcache.h',
  'recordprefetcher.h',
  'tuple.h',
  'twig.h',
  'utils.h',
  'writer.h',
  'hipoeventiterator.h',
  'threadpool.hpp',
  'progresstracker.hpp',
  'chain.h',
)

# Core deps. ROOT is optional: when found, datastreamROOT (a TFile-backed
# datastream) is compiled into libhipo4, giving the chain native support for
# root:// / http:// / https:// / davix:// URLs.
hipo_lib_deps = [ lz4_dep, fmt_dep ]
# -D__LZ4__ enables the LZ4 codepath inside hipo::record (and silences the
# startup banner about "library was compiled without LZ4 support"). The lz4
# dep is already in the link line; this just flips the #ifdef in record.cpp.
hipo_lib_compile_args = [ '-D__LZ4__' ]
if ROOT_dep.found()
  hipo_lib_deps += ROOT_dep
  hipo_lib_compile_args += '-D__ROOT__'
endif

hipo_lib = library(
  meson.project_name(),
  hipo_sources,
  dependencies:   hipo_lib_deps,
  cpp_args:       hipo_lib_compile_args,
  install:        true,
)

install_headers(hipo_headers, subdir: meson.project_name())
project_libs += hipo_lib

hipo_incdir = meson.current_source_dir()

hipo_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with: hipo_lib,
  dependencies: [fmt_dep],
)
