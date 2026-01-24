# Example programs for learning the framework
# These demonstrate different aspects of the framework API

# Common dependencies for all examples
example_deps = [framework_dep, clas12_dep, algorithms_dep, cxxopts_dep, root_dep]

# 01: Simple algorithm example - basic CRTP pattern
executable(
  'example_simple_algorithm',
  files('01_simple_algorithm/example_simple_algorithm.cpp'),
  dependencies: example_deps,
)

# 02: Application builder pattern - fluent API
executable(
  'example_builder',
  files('02_application_builder/example_builder.cpp'),
  dependencies: example_deps,
)

# 03: Histogram auto-fill - automatic histogram management
executable(
  'example_auto_histograms',
  files('03_histograms/example_auto_histograms.cpp'),
  dependencies: example_deps,
)

# 04: Full framework example - comprehensive usage
executable(
  'framework_example',
  files('04_framework/example_framework.cpp'),
  dependencies: example_deps,
)

# 05: Advanced features
executable(
  'advanced',
  files('05_advanced/advanced.cpp'),
  dependencies: example_deps,
)

executable(
  'example_modern_electron_analysis',
  files('05_advanced/example_modern_electron_analysis.cpp'),
  dependencies: example_deps,
)
