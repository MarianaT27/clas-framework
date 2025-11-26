#!/usr/bin/env python3
"""
CLAS12 Analysis Framework - Batch Job Submission Tool

Submit analysis jobs to HPC batch systems (SLURM, PBS, Condor)
"""

import argparse
import json
import os
import sys
import subprocess
from pathlib import Path
from typing import List, Dict, Optional
import glob


class BatchSubmitter:
    """Batch job submission manager"""

    def __init__(self, config_file: str):
        """Initialize from configuration file"""
        self.config = self.load_config(config_file)
        self.validate_config()

    def load_config(self, config_file: str) -> Dict:
        """Load JSON configuration file"""
        with open(config_file, 'r') as f:
            return json.load(f)

    def validate_config(self):
        """Validate configuration"""
        required = ['job_name', 'executable', 'input_files', 'output_dir']
        for key in required:
            if key not in self.config:
                raise ValueError(f"Missing required config key: {key}")

        # Set defaults
        self.config.setdefault('system', 'slurm')
        self.config.setdefault('files_per_job', 10)
        self.config.setdefault('n_cores', 1)
        self.config.setdefault('memory_gb', 4)
        self.config.setdefault('time_limit', '24:00:00')
        self.config.setdefault('log_dir', './logs')

    def expand_input_files(self) -> List[str]:
        """Expand glob patterns in input files"""
        all_files = []

        for pattern in self.config['input_files']:
            if isinstance(pattern, str):
                # Single file or glob pattern
                files = glob.glob(pattern)
                all_files.extend(files)
            elif isinstance(pattern, list):
                # List of files
                all_files.extend(pattern)

        return sorted(all_files)

    def split_files(self, files: List[str], files_per_job: int) -> List[List[str]]:
        """Split files into chunks"""
        return [files[i:i + files_per_job]
                for i in range(0, len(files), files_per_job)]

    def generate_slurm_script(self, job_id: int, input_files: List[str]) -> str:
        """Generate SLURM submission script"""
        cfg = self.config
        script = f"""#!/bin/bash
#SBATCH --job-name={cfg['job_name']}_{job_id}
#SBATCH --output={cfg['log_dir']}/{cfg['job_name']}_{job_id}_%j.out
#SBATCH --error={cfg['log_dir']}/{cfg['job_name']}_{job_id}_%j.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task={cfg['n_cores']}
#SBATCH --mem={cfg['memory_gb']}G
#SBATCH --time={cfg['time_limit']}
"""

        if 'partition' in cfg:
            script += f"#SBATCH --partition={cfg['partition']}\n"

        if 'email' in cfg:
            script += f"#SBATCH --mail-user={cfg['email']}\n"
            mail_types = []
            if cfg.get('email_on_start'): mail_types.append('BEGIN')
            if cfg.get('email_on_end'): mail_types.append('END')
            if cfg.get('email_on_fail'): mail_types.append('FAIL')
            if mail_types:
                script += f"#SBATCH --mail-type={','.join(mail_types)}\n"

        script += "\n"

        # Modules
        for module in cfg.get('modules', []):
            script += f"module load {module}\n"

        # ROOT setup
        if 'root_setup' in cfg:
            script += f"{cfg['root_setup']}\n"

        # Environment
        for env in cfg.get('env_vars', []):
            script += f"export {env}\n"

        script += """
echo "Job started at: $(date)"
echo "Running on node: $(hostname)"
echo "Job ID: $SLURM_JOB_ID"
echo "Number of cores: $SLURM_CPUS_PER_TASK"

"""

        # Create output directory
        output_dir = f"{cfg['output_dir']}/job_{job_id}"
        script += f"mkdir -p {output_dir}\n"
        script += f"cd {output_dir}\n\n"

        # Process each input file
        for input_file in input_files:
            script += f"echo \"Processing: {input_file}\"\n"
            script += f"{cfg['executable']} {input_file}"

            # Add extra arguments
            for arg in cfg.get('extra_args', []):
                script += f" {arg}"

            script += "\n"
            script += """if [ $? -ne 0 ]; then
    echo "ERROR: Failed to process """ + input_file + """"
    exit 1
fi

"""

        script += 'echo "Job completed at: $(date)"\n'

        return script

    def generate_pbs_script(self, job_id: int, input_files: List[str]) -> str:
        """Generate PBS submission script"""
        cfg = self.config
        script = f"""#!/bin/bash
#PBS -N {cfg['job_name']}_{job_id}
#PBS -o {cfg['log_dir']}/{cfg['job_name']}_{job_id}.out
#PBS -e {cfg['log_dir']}/{cfg['job_name']}_{job_id}.err
#PBS -l nodes=1:ppn={cfg['n_cores']}
#PBS -l mem={cfg['memory_gb']}gb
#PBS -l walltime={cfg['time_limit']}
"""

        if 'queue' in cfg:
            script += f"#PBS -q {cfg['queue']}\n"

        if 'email' in cfg:
            script += f"#PBS -M {cfg['email']}\n"
            mail_options = ""
            if cfg.get('email_on_start'): mail_options += "b"
            if cfg.get('email_on_end'): mail_options += "e"
            if cfg.get('email_on_fail'): mail_options += "a"
            if mail_options:
                script += f"#PBS -m {mail_options}\n"

        script += "\ncd $PBS_O_WORKDIR\n\n"

        # Modules
        for module in cfg.get('modules', []):
            script += f"module load {module}\n"

        # ROOT setup
        if 'root_setup' in cfg:
            script += f"{cfg['root_setup']}\n"

        # Environment
        for env in cfg.get('env_vars', []):
            script += f"export {env}\n"

        script += """
echo "Job started at: $(date)"
echo "Running on node: $(hostname)"
echo "Job ID: $PBS_JOBID"

"""

        # Create output directory
        output_dir = f"{cfg['output_dir']}/job_{job_id}"
        script += f"mkdir -p {output_dir}\n"
        script += f"cd {output_dir}\n\n"

        # Process each input file
        for input_file in input_files:
            script += f"echo \"Processing: {input_file}\"\n"
            script += f"{cfg['executable']} {input_file}"

            for arg in cfg.get('extra_args', []):
                script += f" {arg}"

            script += "\n"
            script += """if [ $? -ne 0 ]; then
    echo "ERROR: Failed to process """ + input_file + """"
    exit 1
fi

"""

        script += 'echo "Job completed at: $(date)"\n'

        return script

    def write_script(self, filename: str, content: str):
        """Write script to file and make executable"""
        Path(filename).parent.mkdir(parents=True, exist_ok=True)

        with open(filename, 'w') as f:
            f.write(content)

        # Make executable
        os.chmod(filename, 0o755)

    def submit_jobs(self, dry_run: bool = False) -> List[str]:
        """Submit jobs to batch system"""
        # Get input files
        input_files = self.expand_input_files()
        print(f"Found {len(input_files)} input files")

        # Split into chunks
        file_chunks = self.split_files(input_files, self.config['files_per_job'])
        print(f"Creating {len(file_chunks)} jobs ({self.config['files_per_job']} files per job)")

        # Create directories
        Path(self.config['output_dir']).mkdir(parents=True, exist_ok=True)
        Path(self.config['log_dir']).mkdir(parents=True, exist_ok=True)
        Path('./batch_scripts').mkdir(parents=True, exist_ok=True)

        system = self.config['system'].lower()
        job_ids = []

        # Generate and submit scripts
        for i, chunk in enumerate(file_chunks):
            if system == 'slurm':
                script_content = self.generate_slurm_script(i, chunk)
                script_file = f'./batch_scripts/job_{i}.slurm'
                submit_cmd = f'sbatch {script_file}'
            elif system == 'pbs':
                script_content = self.generate_pbs_script(i, chunk)
                script_file = f'./batch_scripts/job_{i}.pbs'
                submit_cmd = f'qsub {script_file}'
            else:
                print(f"Unknown system: {system}")
                continue

            # Write script
            self.write_script(script_file, script_content)
            print(f"Generated: {script_file}")

            if not dry_run:
                # Submit job
                try:
                    result = subprocess.run(
                        submit_cmd,
                        shell=True,
                        check=True,
                        capture_output=True,
                        text=True
                    )
                    job_id = result.stdout.strip()
                    job_ids.append(job_id)
                    print(f"  Submitted: {job_id}")
                except subprocess.CalledProcessError as e:
                    print(f"  ERROR: Failed to submit {script_file}")
                    print(f"  {e.stderr}")

        if dry_run:
            print("\nDry run complete. Scripts generated but not submitted.")
            print(f"To submit, run: python {sys.argv[0]} {sys.argv[1]} --submit")

        return job_ids


def main():
    parser = argparse.ArgumentParser(
        description='Submit CLAS12 analysis jobs to batch system'
    )
    parser.add_argument(
        'config',
        help='JSON configuration file'
    )
    parser.add_argument(
        '--submit',
        action='store_true',
        help='Actually submit jobs (default is dry-run)'
    )
    parser.add_argument(
        '--files-per-job',
        type=int,
        help='Override files per job from config'
    )

    args = parser.parse_args()

    try:
        # Load configuration
        submitter = BatchSubmitter(args.config)

        # Override if specified
        if args.files_per_job:
            submitter.config['files_per_job'] = args.files_per_job

        # Submit jobs
        job_ids = submitter.submit_jobs(dry_run=not args.submit)

        if args.submit and job_ids:
            print(f"\nSubmitted {len(job_ids)} jobs successfully")

            # Save job IDs
            with open('submitted_jobs.txt', 'w') as f:
                for job_id in job_ids:
                    f.write(f"{job_id}\n")
            print("Job IDs saved to: submitted_jobs.txt")

    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
