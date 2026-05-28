hipo_sources = files(
  'bank.cpp',
  'bybankrecord.cpp',
  'bypagerecord.cpp',
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
  'bybankrecord.h',
  'bypagerecord.h',
  'constants.h',
  'datastream.h',
  'dictionary.h',
  'event.h',
  'fizika.h',
  'fusion.h',
  'hipoexceptions.h',
  'json.h',
  'jsonutil.h',
  'node.h',
  'parser.h',
  'reaction.h',
  'reader.h',
  'record.h',
  'recordbuilder.h',
  'tuple.h',
  'twig.h',
  'utils.h',
  'writer.h',
  'hipoeventiterator.h',
  'threadpool.hpp',
  'progresstracker.hpp',
  'chain.h',
)

hipo_lib = library(
  meson.project_name(),
  hipo_sources,
  dependencies: [ lz4_dep, fmt_dep],
  # -D__LZ4__ enables the LZ4 codepath inside hipo::record / bypagerecord /
  # bybankrecord. Without it every compressed record decodes to 0 events and
  # the reader prints the "compiled without LZ4 support" banner. liblz4 is
  # already linked via lz4_dep; this macro just gates the #ifdef'd code.
  cpp_args: [ '-D__LZ4__' ],
  install: true
)

install_headers(hipo_headers, subdir: meson.project_name())
project_libs += hipo_lib

hipo_dep = declare_dependency(
  # Expose both `extern/` (so callers can do `#include <hipo4/bank.h>`) and
  # `extern/hipo4/` (so legacy `#include <bank.h>` still resolves). Lets us
  # migrate consumers piecemeal without breaking the build.
  include_directories: [include_directories('..'), include_directories('.')],
  link_with: hipo_lib,
  dependencies: [fmt_dep],
)
