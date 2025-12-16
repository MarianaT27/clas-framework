# Batch System

Generic batch job submission for any CLAS12 analysis executable.

## Quick Start

```bash
# Build the batch tool
meson compile -C builddir

# Submit any analysis to SLURM (dry-run by default)
./builddir/batch ./builddir/pi0 --target=LD2 --polarity=Outbending

# Actually submit
./builddir/batch ./builddir/pi0 --target=LD2 --polarity=Outbending --submit

# Use a config file
./builddir/batch --config=config/batch_job_example.json
```

## Usage

```
./batch <executable> [options]

Options:
  --executable      Path to analysis executable (required)
  --config          JSON configuration file
  --data-dir        Data directory with .hipo files
  --target          Target type (LD2, CxC, CuSn)
  --polarity        Magnetic field polarity (Inbending, Outbending)
  --job-name        Job name prefix
  --system          Batch system: slurm, pbs, condor, local (default: slurm)
  --files-per-job   Input files per job (default: 10)
  --num-threads     Threads per job (default: 8)
  --memory          Memory per job in GB (default: 16)
  --time            Time limit HH:MM:SS (default: 12:00:00)
  --partition       SLURM partition / PBS queue (default: production)
  --output-dir      Output directory (default: ./output)
  --log-dir         Log directory (default: ./logs)
  --email           Email for notifications
  --submit          Actually submit jobs (default: dry-run)
  --extra-args      Extra arguments for executable (comma-separated)
```

## Supported Systems

| System | Flag | Submit Command | Monitor |
|--------|------|----------------|---------|
| SLURM | `--system=slurm` | `sbatch` | `squeue -u $USER` |
| PBS | `--system=pbs` | `qsub` | `qstat -u $USER` |
| HTCondor | `--system=condor` | `condor_submit` | `condor_q` |
| Local | `--system=local` | `bash` | `ps aux` |

## Examples

### Pi0 Analysis on SLURM

The `pi0` executable uses the ApplicationBuilder pattern for π⁰ reconstruction:

```bash
./builddir/batch ./builddir/pi0 \
  --target=LD2 \
  --polarity=Outbending \
  --files-per-job=10 \
  --num-threads=8 \
  --memory=16 \
  --time=12:00:00 \
  --submit
```

Pi0-specific options (passed via `--extra-args`):

- `--mass_window=0.2` - Pi0 mass window in GeV (default: 0.2)
- `--min_energy=0.3` - Minimum photon energy in GeV (default: 0.3)

### Electron Analysis with Custom Data

```bash
./builddir/batch ./builddir/electron_analysis \
  --data-dir=/volatile/clas12/users/myname/data \
  --output-dir=/volatile/clas12/users/myname/output \
  --email=myname@jlab.org \
  --submit
```

### Lambda Analysis on PBS

```bash
./builddir/batch ./builddir/lambda \
  --system=pbs \
  --target=CxC \
  --polarity=Inbending \
  --partition=standard \
  --submit
```

### Local Testing

```bash
./builddir/batch ./builddir/improved \
  --system=local \
  --data-dir=./data \
  --files-per-job=1 \
  --submit
```

## Configuration Files

### Example: `config/batch_job_example.json`

```json
{
  "job_name": "clas12_analysis",
  "executable": "./builddir/improved",
  "system": "slurm",
  "data_dir": "/work/clas12/data/RG-A/",
  "output_dir": "./batch_output",
  "log_dir": "./batch_logs",
  "files_per_job": 10,
  "n_cores": 4,
  "memory_gb": 8,
  "time_limit": "12:00:00",
  "partition": "production",
  "email": "user@university.edu"
}
```

Command-line arguments override config file values.

## Workflow

1. **Build your analysis**
   ```bash
   meson compile -C builddir
   ```

2. **Test locally first**
   ```bash
   ./builddir/pi0 data/test.hipo
   ```

3. **Dry run** (generates scripts without submitting)
   ```bash
   ./builddir/batch ./builddir/pi0 --target=LD2 --polarity=Outbending
   ls batch_scripts/
   ```

4. **Submit jobs**
   ```bash
   ./builddir/batch ./builddir/pi0 --target=LD2 --polarity=Outbending --submit
   ```

5. **Monitor progress**
   ```bash
   squeue -u $USER
   tail -f logs/*.out
   ```

6. **Merge results**
   ```bash
   hadd -f merged.root output/*/*.root
   ```

## Output Structure

```
batch_scripts/
├── job_0.slurm
├── job_1.slurm
└── ...

output/
├── job_0/
│   └── analysis.root
├── job_1/
│   └── analysis.root
└── ...

logs/
├── job_0_12345.out
├── job_0_12345.err
└── ...
```

## Tips

!!! tip "Files per job"
    5-20 is usually optimal. Too few = overhead, too many = long jobs.

!!! tip "Memory"
    ~2GB base + 1GB per thread for typical analyses.

!!! tip "Time limit"
    Add 20% buffer to estimated runtime.

!!! warning "Always test locally"
    Test your analysis locally before submitting batch jobs.

!!! tip "Check output"
    Check one job's output before merging all results.
