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
  # framework_inc (include-only, no link) lets local_batch reuse the header-only
  # perf:: helpers (parse_perf_line / render_table) to aggregate per-slice timing.
  include_directories: framework_inc,
  dependencies: [hipo4_dep, fmt_dep, cxxopts_dep, toml_dep],
)

# Chain-only electron-selection throughput benchmark. Uses ONLY hipo::chain
# (no framework) and replicates ElectronSelectionAlg's cuts, so its events/sec
# is the ceiling the full framework is compared against. See
# compare_electron_benchmark.sh.
executable(
  'benchmark_electron',
  files('benchmark_electron.cpp'),
  dependencies: [hipo4_dep, fmt_dep, cxxopts_dep],
)
