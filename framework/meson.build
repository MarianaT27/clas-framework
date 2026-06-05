# Framework library - algorithm/service infrastructure built on top of
# the clas12 data types. The framework depends on clas12_dep; there is no
# longer a direct include-path hack reaching across the boundary.

framework_inc = include_directories('include')

framework_sources = files(
  'src/ApplicationBuilder.cpp',
  'src/Service.cpp',
  'src/services/DecayTreeFitterService.cpp',
  'src/services/HistogramService.cpp',
  'src/services/LoggingService.cpp',
  'src/services/RunConditionsService.cpp',
)

framework_deps = [clas12_dep, fmt_dep, toml_dep, root_dep, hipo4_dep, eigen_dep, mh_dep, pfr_dep, sqlite3_dep]
framework_cpp_args = []
if have_mysql
  framework_deps += mysql_dep
  framework_cpp_args += '-DHAVE_MYSQL'
endif

framework_lib = library(
  'clas12-framework',
  framework_sources,
  include_directories: framework_inc,
  dependencies: framework_deps,
  cpp_args: framework_cpp_args,
  install: true,
)

framework_dep = declare_dependency(
  include_directories: framework_inc,
  link_with: framework_lib,
  dependencies: [clas12_dep, fmt_dep, toml_dep, hipo4_dep, eigen_dep, mh_dep, pfr_dep],
)

install_subdir('include/clas12', install_dir: get_option('includedir'))
