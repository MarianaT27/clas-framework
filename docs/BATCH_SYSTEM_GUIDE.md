# Batch Job Submission System - User Guide

## Overview

The CLAS12 Analysis Framework includes a comprehensive batch job submission system for running analyses on HPC clusters. It supports SLURM, PBS, and HTCondor batch systems.

## Quick Start

### 1. Prepare Your Configuration

Create a JSON configuration file (see `config/batch_job_example.json`):

```json
{
  "job_name": "my_analysis",
  "executable": "./build/main_improved",
  "system": "slurm",
  "input_files": ["/data/clas12/*.hipo"],
  "output_dir": "./output",
  "log_dir": "./logs",
  "files_per_job": 10,
  "n_cores": 4,
  "memory_gb": 8,
  "time_limit": "12:00:00"
}
```

### 2. Submit Jobs

```bash
# Dry run (generate scripts but don't submit)
./scripts/submit_batch.py config/my_job.json

# Actually submit jobs
./scripts/submit_batch.py config/my_job.json --submit
```

### 3. Monitor Progress

```bash
# Monitor continuously (updates every 60 seconds)
./scripts/monitor_jobs.py

# Check once
./scripts/monitor_jobs.py --once

# Custom interval
./scripts/monitor_jobs.py --interval 120
```

### 4. Merge Results

```bash
# Merge all ROOT files
./scripts/merge_outputs.py --input-dir ./output --output merged.root

# Merge by type (histograms, ntuples, etc.)
./scripts/merge_outputs.py --input-dir ./output --by-type --merged-dir ./merged
```

## Configuration Options

### Required Fields

| Field | Description | Example |
|-------|-------------|---------|
| `job_name` | Name for the job | `"electron_analysis"` |
| `executable` | Path to analysis executable | `"./build/main_improved"` |
| `input_files` | List of input files (supports globs) | `["/data/*.hipo"]` |
| `output_dir` | Output directory | `"./output"` |

### Resource Configuration

| Field | Default | Description |
|-------|---------|-------------|
| `system` | `"slurm"` | Batch system: `slurm`, `pbs`, or `local` |
| `files_per_job` | `10` | Number of input files per job |
| `n_cores` | `1` | Number of CPU cores per job |
| `memory_gb` | `4` | Memory in GB per job |
| `time_limit` | `"24:00:00"` | Wall time limit (HH:MM:SS) |
| `partition` | `"default"` | SLURM partition |
| `queue` | `"default"` | PBS queue |

### Environment Setup

| Field | Description | Example |
|-------|-------------|---------|
| `root_setup` | Command to setup ROOT | `"source /apps/root/bin/thisroot.sh"` |
| `modules` | Modules to load | `["gcc/11", "python/3.9"]` |
| `env_vars` | Environment variables | `["OMP_NUM_THREADS=4"]` |

### Additional Options

| Field | Description |
|-------|-------------|
| `extra_args` | Extra arguments for executable |
| `email` | Email for notifications |
| `email_on_start` | Send email when job starts |
| `email_on_end` | Send email when job ends |
| `email_on_fail` | Send email on failure |

## Examples

### Example 1: SLURM Cluster (JLab)

```json
{
  "job_name": "rg-a_electron",
  "executable": "./build/main_improved",
  "system": "slurm",

  "input_files": [
    "/volatile/clas12/users/myname/data/rg-a/run_*.hipo"
  ],

  "output_dir": "/volatile/clas12/users/myname/output",
  "log_dir": "/volatile/clas12/users/myname/logs",

  "files_per_job": 10,
  "n_cores": 4,
  "memory_gb": 8,
  "time_limit": "12:00:00",
  "partition": "production",

  "root_setup": "source /apps/root/6.28.04/setenv_root.sh",

  "modules": [
    "gcc/11.2.0"
  ],

  "extra_args": [
    "--max-events", "0",
    "--num-threads", "4"
  ],

  "email": "myname@jlab.org",
  "email_on_end": true,
  "email_on_fail": true
}
```

Submit:
```bash
./scripts/submit_batch.py config/jlab_job.json --submit
```

### Example 2: PBS System

```json
{
  "job_name": "analysis",
  "executable": "./build/main_improved",
  "system": "pbs",

  "input_files": ["/data/*.hipo"],
  "output_dir": "./output",
  "log_dir": "./logs",

  "files_per_job": 5,
  "n_cores": 2,
  "memory_gb": 4,
  "time_limit": "24:00:00",
  "queue": "standard"
}
```

### Example 3: Local Testing

```json
{
  "job_name": "test",
  "executable": "./build/main_improved",
  "system": "local",

  "input_files": ["./test_data/*.hipo"],
  "output_dir": "./test_output",
  "files_per_job": 1,

  "extra_args": ["--max-events", "1000"]
}
```

Run locally:
```bash
./scripts/submit_batch.py config/local_test.json --submit
# Scripts generated in ./batch_scripts/
bash ./batch_scripts/job_0.sh
```

## Workflow

### Complete Analysis Workflow

1. **Prepare Analysis**
   ```bash
   # Build your analysis
   meson compile -C build

   # Test locally first
   ./build/main_improved test_data.hipo
   ```

2. **Configure Batch Jobs**
   ```bash
   # Copy example config
   cp config/batch_job_example.json config/my_analysis.json

   # Edit configuration
   vim config/my_analysis.json
   ```

3. **Dry Run**
   ```bash
   # Generate scripts without submitting
   ./scripts/submit_batch.py config/my_analysis.json

   # Check generated scripts
   ls batch_scripts/
   cat batch_scripts/job_0.slurm
   ```

4. **Submit Jobs**
   ```bash
   # Submit to cluster
   ./scripts/submit_batch.py config/my_analysis.json --submit

   # Job IDs saved to submitted_jobs.txt
   ```

5. **Monitor Progress**
   ```bash
   # Watch job status
   ./scripts/monitor_jobs.py --interval 120

   # Or check manually
   squeue -u $USER  # SLURM
   qstat -u $USER   # PBS
   ```

6. **Check Logs**
   ```bash
   # View job logs
   tail -f logs/my_analysis_0_*.out

   # Check for errors
   grep -i error logs/*.err
   ```

7. **Merge Results**
   ```bash
   # Merge all outputs
   ./scripts/merge_outputs.py \
     --input-dir ./output \
     --by-type \
     --merged-dir ./final_results

   # Check merged file
   root -l final_results/merged_all.root
   ```

## Tips and Best Practices

### Resource Allocation

- **CPU Cores**: Match `n_cores` with `--num-threads` in `extra_args`
- **Memory**: Estimate ~2GB base + 1GB per thread for typical analyses
- **Time Limit**: Add 20% buffer to estimated runtime
- **Files per Job**: Balance between:
  - Too few: Many short jobs (overhead)
  - Too many: Long jobs (harder to checkpoint)
  - Sweet spot: 5-20 files per job

### Testing

1. Always test locally first
2. Run a single job before submitting many
3. Use small `--max-events` for initial tests
4. Check one job's output before merging all

### Debugging

If jobs fail:

1. **Check logs**
   ```bash
   cat logs/job_name_0_*.err
   ```

2. **Test interactively**
   ```bash
   # On SLURM
   salloc --partition=production --ntasks=1 --cpus-per-task=4 --mem=8G
   cd $SLURM_SUBMIT_DIR
   source /apps/root/bin/thisroot.sh
   ./build/main_improved input_file.hipo
   ```

3. **Check common issues**
   - ROOT not in PATH
   - Missing input files
   - Insufficient memory
   - Permissions issues

### Performance Optimization

1. **Parallel Processing**
   - Use multi-threading within jobs
   - Set `OMP_NUM_THREADS` environment variable
   - Match cores requested to threads used

2. **I/O Optimization**
   - Use local scratch space for temporary files
   - Avoid writing many small files
   - Merge outputs in batches if needed

3. **Job Array Size**
   - Some systems limit array size
   - Use `max_concurrent_jobs` if needed
   - Consider splitting very large submissions

## Troubleshooting

### Common Errors

**Error: Command not found**
```bash
# Solution: Make sure scripts are executable
chmod +x scripts/*.py
```

**Error: hadd not found**
```bash
# Solution: Load ROOT environment
source /path/to/root/bin/thisroot.sh
```

**Error: Permission denied**
```bash
# Solution: Check directory permissions
chmod 755 ./output ./logs
```

**Job stuck in PENDING**
- Check queue limits: `squeue -u $USER`
- Verify partition exists: `sinfo`
- Check resource availability

**Out of memory errors**
- Increase `memory_gb` in config
- Reduce number of threads
- Process fewer events per job

### Getting Help

- Check batch system documentation:
  - SLURM: `man sbatch`, `man squeue`
  - PBS: `man qsub`, `man qstat`
- Contact your cluster support team
- Check job logs for error messages

## Advanced Features

### Custom Job Scripts

You can modify the generated scripts before submission:

```bash
# Generate scripts
./scripts/submit_batch.py config/my_job.json

# Edit a script
vim batch_scripts/job_0.slurm

# Submit manually
sbatch batch_scripts/job_0.slurm
```

### Job Dependencies

For SLURM, you can create job dependencies:

```bash
# Submit first job
JOB1=$(sbatch batch_scripts/job_0.slurm | awk '{print $4}')

# Submit dependent job
sbatch --dependency=afterok:$JOB1 merge_job.slurm
```

### Checkpointing

Add checkpointing to your analysis for long jobs:

```cpp
// In your main program
app.set_checkpoint_interval(10000);  // Save every 10k events
app.set_checkpoint_file("checkpoint.root");
```

### Resource Monitoring

Add resource monitoring to job scripts:

```bash
# In SLURM script
echo "CPU usage:"
sstat -j $SLURM_JOB_ID --format=AveCPU,MaxRSS

# At job end
sacct -j $SLURM_JOB_ID --format=JobID,MaxRSS,Elapsed
```

## Summary

The batch system provides:

✅ **Easy submission** - Simple JSON configuration
✅ **Multiple systems** - SLURM, PBS, Condor support
✅ **Monitoring** - Track job progress
✅ **Output merging** - Combine results automatically
✅ **Flexible** - Customize for your needs

For questions or issues, check the documentation in `docs/` or contact your system administrator.

---

**Happy batch processing!** 🚀
