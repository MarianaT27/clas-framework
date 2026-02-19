hipo4_sources = files(
  'schema.cpp',
  'dictionary.cpp',
  'io.cpp',
  'file_sink.cpp',
  'record.cpp',
  'record_builder.cpp',
  'reader.cpp',
  'writer.cpp',
  'chain.cpp',
  'validator.cpp',
)

hipo4_headers = files(
  'result.h',
  'constants.h',
  'schema.h',
  'dictionary.h',
  'io.h',
  'file_sink.h',
  'record.h',
  'record_builder.h',
  'bank_view.h',
  'bank.h',
  'event_builder.h',
  'reader.h',
  'writer.h',
  'column_stats.h',
  'chain.h',
  'validator.h',
  'threadpool.hpp',
  'progresstracker.hpp',
)

hipo4_lib = library(
  'hipo4',
  hipo4_sources,
  dependencies: [lz4_dep, zlib_dep, fmt_dep, thread_dep],
  include_directories: include_directories('.'),
  cpp_args: ['-D__LZ4__'],
  install: true,
)

install_headers(hipo4_headers, subdir: 'hipo4')

hipo4_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with: hipo4_lib,
  dependencies: [fmt_dep],
)

project_libs += hipo4_lib
