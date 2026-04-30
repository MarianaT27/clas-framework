# Batch system (SWIF2)

The framework's batch tool submits CLAS12 analyses to the JLab cluster via
[SWIF2](https://scicomp.jlab.org/docs/swif), which wraps Auger and handles
tape staging from `/mss`.

The flow is:

1. You write a single **farm TOML** that points at your analysis TOML and
   declares SWIF2 resources + JLab paths.
2. `./builddir/batch <farm.toml>` slices your input file list, synthesizes
   one **per-job TOML** per chunk (overriding `[run].input`/`[run].output`),
   and emits a workflow shell script.
3. SWIF2 stages each job's input HIPO files from `/mss` into the worker
   node's working directory, runs the analysis against the synthesized TOML,
   and copies the output ROOT file and log back to `/volatile`.

The job script just runs `executable --config job.toml`. No manual `cp`
glue, no `cd`-to-output gymnastics.

## Quick start

```bash
# Build the batch tool
meson compile -C builddir

# Dry-run — synthesizes per-job TOMLs and the swif2 script under batch_scripts/,
# prints a summary, prints a sample synthesized TOML.
./builddir/batch config/electron_analysis.farm.toml

# Submit on the JLab login node
./builddir/batch config/electron_analysis.farm.toml --submit
```

## Farm TOML schema

```toml
[farm]
analysis_config = "config/electron_analysis.toml"   # base analysis TOML
executable      = "builddir/electron_analysis"
files_per_job   = 10

# May be a /mss path, a /cache path, a directory (recursive), or an array.
inputs          = "/mss/clas12/rg-d/production/pass1/recon/LD2/dst/train/SIDIS/"

# Where per-job result.root and log.txt land via SWIF2 -output.
output_dir      = "/volatile/clas12/users/me/electron_analysis"
log_dir         = "/volatile/clas12/users/me/electron_analysis/logs"

[farm.swif2]
workflow   = "electron_analysis_LD2_outbending"
# `site` is only for cross-site workflows that move data between JLab and an
# external compute facility (those require globus + login config). Leave it
# unset for normal in-house JLab submission.
# site     = ""
account    = "clas12"
partition  = "production"
cores      = 8
ram_gb     = 16
disk_gb    = 50
time       = "12:00:00"
constraint = ""                                     # optional, e.g. "el9"

[farm.swif2.modules]
load = ["gcc/13.2.0", "root/6.36.04"]

[farm.swif2.tags]
stream  = "electron"
target  = "LD2"

[farm.env]
ROOT_HIST = "0"
# OMP_NUM_THREADS is set automatically to $SWIF2_JOB_CORES inside the job.
```

`[farm].inputs` is the canonical input source. Three forms are accepted:

- A single string that is a directory: scanned recursively for `*.hipo`.
- A single string that is a file (`/mss/.../run_5032.hipo`): used as-is.
- An array of strings: each entry is treated as a directory or file.

`[farm.swif2]` resources are passed straight through to SWIF2 via
`-cores`, `-ram`, `-disk`, `-time`, `-account`, `-partition`,
`-constraint`. Tags become `-tag KEY VALUE` arguments.

## Per-job TOML

For chunk `i`, the batch tool writes
`batch_scripts/<workflow>_job_<i>.toml`:

```toml
[run]
input       = ["input_0.hipo", "input_1.hipo", "input_2.hipo", ..., "input_9.hipo"]
output      = "result.root"
num_threads = 8                # propagated from [farm.swif2].cores
percentage  = 100.0

# [analysis] and per-algorithm sections are copied verbatim from
# the base analysis TOML.
[analysis]
target      = "LD2"
polarity    = "Outbending"
beam_energy = 10.53

[ElectronAnalysis]
enable_chi2pid_cut = true
# ...
```

Note that `input` and `output` use **local** filenames, not JLab paths.
SWIF2 stages the original `/mss` files into the job's working directory
under `input_0.hipo`, `input_1.hipo`, etc., and copies `result.root` from
the working directory back to
`<output_dir>/<workflow>_job_<i>/result.root` after the job exits.

## Workflow script

The tool also emits `batch_scripts/<workflow>.swif2.sh`, a single shell
script with one `swif2 add-job` block per chunk:

```bash
#!/usr/bin/env bash
set -euo pipefail

swif2 create -workflow 'electron_analysis_LD2_outbending'

# job 0
swif2 add-job -workflow 'electron_analysis_LD2_outbending' \
    -name 'electron_analysis_LD2_outbending_job_0' \
    -account 'clas12' -partition 'production' \
    -cores 8 -ram 16G -disk 50G -time '12:00:00' \
    -tag 'stream' 'electron' \
    -input 'input_0.hipo' '/mss/clas12/rg-d/.../run_5032_0.hipo' \
    -input 'input_1.hipo' '/mss/clas12/rg-d/.../run_5032_1.hipo' \
    ... \
    -input job.toml '/work/.../batch_scripts/...job_0.toml' \
    -output 'result.root' '/volatile/.../electron_analysis_LD2_outbending_job_0/result.root' \
    -output 'log.txt'     '/volatile/.../logs/electron_analysis_LD2_outbending_job_0.log' \
    -shell 'module load gcc/13.2.0 ...
            ./builddir/electron_analysis --config job.toml 2>&1 | tee log.txt'

# job 1, job 2, ...

swif2 run -workflow 'electron_analysis_LD2_outbending'
```

The script is also kept on disk after submission as a record of what ran.

## CLI

```
./batch <farm.toml> [options]

  --submit               Actually run the swif2 script (default: dry-run)
  --files-per-job=N      Override [farm].files_per_job
  --workflow=NAME        Override [farm.swif2].workflow
  --max-jobs=N           Synthesize at most N jobs (for testing)
  --print-job=N          Print the synthesized TOML for job N and exit
```

`--print-job` is convenient for sanity-checking the synthesis before
submitting:

```bash
./builddir/batch config/electron_analysis.farm.toml --print-job 0
```

## Workflow

1. **Build** — `meson compile -C builddir`.
2. **Write the analysis TOML** — `config/<name>.toml` (the existing schema:
   `[run]`, `[analysis]`, per-algorithm sections).
3. **Test locally** —
   `./builddir/<name> --config config/<name>.toml` against a single HIPO file.
4. **Write the farm TOML** — `config/<name>.farm.toml`.
5. **Dry-run** — `./builddir/batch config/<name>.farm.toml`. Inspect the
   summary, the sample synthesized TOML, and the generated SWIF2 script.
6. **Submit** — add `--submit`. The tool runs the SWIF2 script which
   creates the workflow, adds every job, and starts it.
7. **Monitor** —
    ```bash
    swif2 status -workflow <workflow_name>
    swif2 list-jobs -workflow <workflow_name> -display all
    ```
8. **Merge** — when complete:
    ```bash
    hadd -f merged.root <output_dir>/*/result.root
    ```

## /mss vs /cache

SWIF2 routes `/mss/...` paths through the tape system: it stages each
input from tape to disk before the job runs, prefetching as the workflow
progresses. `/cache/...` paths skip the tape route and pull straight from
the disk cache. Either works; tape is the authoritative source.

## Tips

!!! tip "Files per job"
    5–20 is usually optimal. Too few = scheduling overhead, too many =
    long jobs that retry expensively on transient failures.

!!! tip "Memory"
    Start with ~2 GB base + ~1 GB per thread. Watch SWIF2's memory
    accounting and bump `[farm.swif2].ram_gb` if jobs are killed.

!!! tip "Time limit"
    Add ~20% buffer over your single-file local runtime × `files_per_job`.

!!! warning "Always test locally"
    Run your analysis locally with the same `[run]` / `[analysis]` /
    per-algorithm sections before submitting. Errors in cuts or paths
    won't show up until a job runs.

!!! tip "Re-running failed jobs"
    SWIF2 tracks per-job state. Use
    `swif2 retry-jobs -workflow <name> -problems` to re-submit only the
    jobs that failed.
