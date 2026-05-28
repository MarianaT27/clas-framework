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

# Lean electron-selection baseline (runs only ElectronSelectionAlg) — the
# full-framework counterpart of tools/benchmark_electron for A/B benchmarking.
executable(
  'electron_selection',
  files('electron_selection/electron_selection.cpp'),
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

# CuSn contamination analysis
executable(
  'CuSn_contamination',
  files('CuSn_contamination/CuSn_contamination.cpp', 'CuSn_contamination/CusnContaminationAlg.hpp'),
  dependencies: analysis_deps,
)

# ALERT_track_matching analysis
executable(
  'ALERT_track_matching',
  files('ALERT_track_matching/ALERT_track_matching.cpp'),
  dependencies: analysis_deps,
)

# Pi0 multiplicity ratio
executable(
  'pi0_multiplicity',
  files('pi0_multiplicity/pi0_multiplicity.cpp'),
  dependencies: analysis_deps,
)

# Pi0 multiplicity skim (event filter)
executable(
  'pi0_multiplicity_skim',
  files('pi0_multiplicity_skim/pi0_multiplicity_skim.cpp'),
  dependencies: analysis_deps,
)

# AI hadron PID analysis
executable(
  'ai_pid_hadron',
  files('ai_pid_hadron/ai_pid_hadron.cpp', 'ai_pid_hadron/AiPidHadronAlg.hpp'),
  dependencies: analysis_deps,
)
