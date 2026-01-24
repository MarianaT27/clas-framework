# User analysis executables
# Each subdirectory contains a specific physics analysis

# Common dependencies for all analyses
analysis_deps = [framework_dep, clas12_dep, algorithms_dep, cxxopts_dep, root_dep]

# Electron analysis
executable(
  'electron_analysis',
  files('electron/electron_analysis.cpp'),
  dependencies: analysis_deps,
)

# Pion analysis
executable(
  'pion_analysis',
  files('pion/pion_analysis.cpp'),
  dependencies: analysis_deps,
)

# Pi0 reconstruction
executable(
  'pi0',
  files('pi0/pi0.cpp'),
  dependencies: analysis_deps,
)

# Lambda analysis
executable(
  'lambda',
  files('lambda/lambda.cpp'),
  dependencies: analysis_deps,
)

executable(
  'decays_lambda',
  files('lambda/decays_lambda.cpp'),
  dependencies: analysis_deps,
)

# Nuclear transparency
executable(
  'nuclear_transparency',
  files('nuclear_transparency/nuclear_transparency.cpp'),
  dependencies: analysis_deps,
)

# Di-pion filter
executable(
  'dipion_filter',
  files('dipion/dipion_filter.cpp'),
  dependencies: analysis_deps,
)
