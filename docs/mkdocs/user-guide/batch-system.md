# Batch System

Generic batch job submission for any CLAS12 analysis executable. Each
analysis takes a single `--config <path.toml>` flag, so the batch tool
just needs to know which executable to run, which TOML config to pass
to each job, and how to slice the input across jobs.

## Quick Start

```bash
# Build the batch tool
meson compile -C builddir

# Submit a TOML-driven analysis to SLURM (dry-run by default)
./builddir/batch ./builddir/pi0 --config config/pi0.toml

# Actually submit
./builddir/batch ./builddir/pi0 --config config/pi0.toml --submit

# Use a JSON batch config (separate from the analysis TOML)
./builddir/batch --config-json=config/batch_job_example.json
```

## Usage

```
./batch <executable> [options]

Options:
  --executable      Path to analysis executable (required)
  --config          Per-job analysis TOML config (required for analyses
                    using AnalysisRunner)
  --config-json     Optional JSON file with batch-tool defaults
  --data-dir        Data directory with .hipo files (overrides
                    [run].input in the analysis TOML)
  --job-name        Job name prefix
  --system          Batch system: slurm, local (default: slurm)
  --files-per-job   Input files per job (default: 10)
  --num-threads     Threads per job (default: 8)
  --memory          Memory per job in GB (default: 16)
  --time            Time limit HH:MM:SS (default: 12:00:00)
  --partition       SLURM partition (default: production)
  --output-dir      Output directory (default: ./output)
  --log-dir         Log directory (default: ./logs)
  --email           Email for notifications
  --submit          Actually submit jobs (default: dry-run)
```

Per-analysis tuning (cuts, enables, mass windows, scan parameters)
lives entirely in the analysis TOML — see the analysis's own example
config under `config/`. The batch tool does not pass per-algorithm
flags through to the executable.

## Supported Systems

| System | Flag | Submit Command | Monitor |
|--------|------|----------------|---------|
| SLURM  | `--system=slurm` | `sbatch` | `squeue -u $USER` |
| Local  | `--system=local` | `bash`   | `ps aux` |

## Examples

### Pi0 Analysis on SLURM

```bash
./builddir/batch ./builddir/pi0 \
  --config config/pi0.toml \
  --files-per-job=10 \
  --num-threads=8 \
  --memory=16 \
  --time=12:00:00 \
  --submit
```

Edit `config/pi0.toml` for analysis-specific knobs:

```toml
[Pi0Reconstruction]
mass_window = 0.2

[PhotonSelection]
min_energy = 0.3
```

### Electron Analysis with Custom Data

```bash
./builddir/batch ./builddir/electron_analysis \
  --config config/electron_analysis.toml \
  --data-dir=/volatile/clas12/users/myname/data \
  --output-dir=/volatile/clas12/users/myname/output \
  --email=myname@jlab.org \
  --submit
```

### Local Testing

```bash
./builddir/batch ./builddir/electron_analysis \
  --config config/electron_analysis.toml \
  --system=local \
  --data-dir=./data \
  --files-per-job=1 \
  --submit
```

## Workflow

1. **Build your analysis** — `meson compile -C builddir`.
2. **Write a TOML config** under `config/<name>.toml`. See `config/electron_analysis.toml` for the schema.
3. **Test locally** — `./builddir/<name> --config config/<name>.toml`.
4. **Dry-run with batch** — `./builddir/batch ./builddir/<name> --config config/<name>.toml`.
5. **Submit** — add `--submit`.
6. **Monitor** — `squeue -u $USER` ; `tail -f logs/*.out`.
7. **Merge** — `hadd -f merged.root output/*/*.root`.

## Tips

!!! tip "Files per job"
    5-20 is usually optimal. Too few = overhead, too many = long jobs.

!!! tip "Memory"
    ~2GB base + 1GB per thread for typical analyses.

!!! tip "Time limit"
    Add 20% buffer to estimated runtime.

!!! warning "Always test locally"
    Test your analysis locally with the same TOML config before
    submitting batch jobs. Mistakes in `[run]` paths or per-algorithm
    sections won't show up until the job runs.
