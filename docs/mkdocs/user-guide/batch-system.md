# Batch System

Submit analysis jobs to computing clusters.

## Quick Start

```bash
# Dry run (generates scripts without submitting)
./builddir/batch ./builddir/pi0 --target=LD2 --polarity=Outbending

# Actually submit
./builddir/batch ./builddir/pi0 --target=LD2 --polarity=Outbending --submit
```

## Supported Systems

| System | Flag | Submit Command |
|--------|------|----------------|
| SLURM | `--system=slurm` | `sbatch` |
| PBS | `--system=pbs` | `qsub` |
| HTCondor | `--system=condor` | `condor_submit` |
| Local | `--system=local` | `bash` |

## Options

| Option | Description | Default |
|--------|-------------|---------|
| `--executable` | Analysis executable | Required |
| `--data-dir` | Data directory | - |
| `--target` | Target type | - |
| `--polarity` | Magnetic field | - |
| `--files-per-job` | Files per job | 10 |
| `--num-threads` | Threads per job | 8 |
| `--memory` | Memory (GB) | 16 |
| `--time` | Time limit | 12:00:00 |
| `--submit` | Actually submit | false |

## Example

```bash
./builddir/batch ./builddir/electron_analysis \
  --data-dir=/volatile/clas12/data \
  --output-dir=/volatile/clas12/output \
  --files-per-job=10 \
  --num-threads=8 \
  --memory=16 \
  --time=12:00:00 \
  --submit
```

## Workflow

1. Build your analysis: `meson compile -C builddir`
2. Test locally: `./builddir/pi0 data/test.hipo`
3. Dry run: `./builddir/batch ./builddir/pi0 --target=LD2`
4. Submit: `./builddir/batch ./builddir/pi0 --target=LD2 --submit`
5. Monitor: `squeue -u $USER`
6. Merge: `hadd -f merged.root output/*/*.root`
