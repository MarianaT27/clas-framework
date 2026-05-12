# Utility tools
# Batch job submission and other helper utilities

# Batch job submitter - works with any analysis executable
executable(
  'batch',
  files('batch.cpp'),
  dependencies: [framework_dep, clas12_dep, hipo4_dep, fmt_dep, cxxopts_dep, root_dep],
)

# Local (non-SWIF2) batch driver — split one big HIPO file across N
# concurrent processes by record range. Works alongside `batch` (SWIF2)
# but targets ifarm interactive nodes where the user wants to overcome
# single-process mutex contention without going through the farm.
executable(
  'local_batch',
  files('local_batch.cpp'),
  dependencies: [hipo4_dep, fmt_dep, cxxopts_dep, toml_dep],
)
