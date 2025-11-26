#!/usr/bin/env python3
"""
Monitor batch job status and progress
"""

import argparse
import subprocess
import time
import sys
from typing import Dict, List
from datetime import datetime


class JobMonitor:
    """Monitor batch jobs"""

    def __init__(self, system: str = 'slurm'):
        self.system = system.lower()

    def get_job_status(self, job_ids: List[str]) -> Dict[str, Dict]:
        """Get status of jobs"""
        if self.system == 'slurm':
            return self._get_slurm_status(job_ids)
        elif self.system == 'pbs':
            return self._get_pbs_status(job_ids)
        else:
            return {}

    def _get_slurm_status(self, job_ids: List[str]) -> Dict[str, Dict]:
        """Get SLURM job status"""
        status = {}

        for job_id in job_ids:
            try:
                # Use scontrol to get detailed info
                result = subprocess.run(
                    ['scontrol', 'show', 'job', job_id],
                    capture_output=True,
                    text=True,
                    check=True
                )

                info = {}
                for line in result.stdout.split('\n'):
                    if '=' in line:
                        parts = line.split('=')
                        key = parts[0].strip()
                        value = '='.join(parts[1:]).strip()
                        info[key] = value

                status[job_id] = {
                    'state': info.get('JobState', 'UNKNOWN'),
                    'start_time': info.get('StartTime', 'N/A'),
                    'run_time': info.get('RunTime', 'N/A'),
                    'node': info.get('NodeList', 'N/A')
                }

            except subprocess.CalledProcessError:
                status[job_id] = {'state': 'NOT_FOUND'}

        return status

    def _get_pbs_status(self, job_ids: List[str]) -> Dict[str, Dict]:
        """Get PBS job status"""
        status = {}

        for job_id in job_ids:
            try:
                result = subprocess.run(
                    ['qstat', '-f', job_id],
                    capture_output=True,
                    text=True,
                    check=True
                )

                info = {}
                for line in result.stdout.split('\n'):
                    if '=' in line:
                        parts = line.strip().split('=', 1)
                        if len(parts) == 2:
                            key = parts[0].strip()
                            value = parts[1].strip()
                            info[key] = value

                status[job_id] = {
                    'state': info.get('job_state', 'UNKNOWN'),
                    'start_time': info.get('start_time', 'N/A'),
                    'resources_used.walltime': info.get('resources_used.walltime', 'N/A'),
                    'exec_host': info.get('exec_host', 'N/A')
                }

            except subprocess.CalledProcessError:
                status[job_id] = {'state': 'NOT_FOUND'}

        return status

    def print_status(self, status: Dict[str, Dict]):
        """Print job status in formatted table"""
        print("\n" + "=" * 80)
        print(f"Job Status Report - {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
        print("=" * 80)

        # Count states
        state_counts = {}
        for job_info in status.values():
            state = job_info.get('state', 'UNKNOWN')
            state_counts[state] = state_counts.get(state, 0) + 1

        print(f"\nTotal jobs: {len(status)}")
        for state, count in sorted(state_counts.items()):
            print(f"  {state}: {count}")

        print("\n" + "-" * 80)
        print(f"{'Job ID':<20} {'State':<15} {'Run Time':<20} {'Node':<20}")
        print("-" * 80)

        for job_id, info in sorted(status.items()):
            state = info.get('state', 'UNKNOWN')
            runtime = info.get('run_time', info.get('resources_used.walltime', 'N/A'))
            node = info.get('node', info.get('exec_host', 'N/A'))

            print(f"{job_id:<20} {state:<15} {runtime:<20} {node:<20}")

        print("=" * 80 + "\n")

    def monitor_loop(self, job_ids: List[str], interval: int = 60):
        """Monitor jobs in a loop"""
        try:
            while True:
                status = self.get_job_status(job_ids)
                self.print_status(status)

                # Check if all jobs are done
                active_states = {'PENDING', 'RUNNING', 'CONFIGURING', 'Q', 'R'}
                active_jobs = [
                    jid for jid, info in status.items()
                    if info.get('state', '') in active_states
                ]

                if not active_jobs:
                    print("All jobs completed!")
                    break

                print(f"Waiting {interval} seconds before next check...")
                time.sleep(interval)

        except KeyboardInterrupt:
            print("\nMonitoring stopped by user")


def main():
    parser = argparse.ArgumentParser(
        description='Monitor CLAS12 batch jobs'
    )
    parser.add_argument(
        '--job-file',
        default='submitted_jobs.txt',
        help='File containing job IDs (one per line)'
    )
    parser.add_argument(
        '--system',
        choices=['slurm', 'pbs'],
        default='slurm',
        help='Batch system type'
    )
    parser.add_argument(
        '--interval',
        type=int,
        default=60,
        help='Monitoring interval in seconds'
    )
    parser.add_argument(
        '--once',
        action='store_true',
        help='Check status once and exit'
    )

    args = parser.parse_args()

    # Read job IDs
    try:
        with open(args.job_file, 'r') as f:
            job_ids = [line.strip() for line in f if line.strip()]
    except FileNotFoundError:
        print(f"ERROR: Job file not found: {args.job_file}", file=sys.stderr)
        sys.exit(1)

    if not job_ids:
        print("ERROR: No job IDs found", file=sys.stderr)
        sys.exit(1)

    print(f"Monitoring {len(job_ids)} jobs...")

    monitor = JobMonitor(args.system)

    if args.once:
        status = monitor.get_job_status(job_ids)
        monitor.print_status(status)
    else:
        monitor.monitor_loop(job_ids, args.interval)


if __name__ == '__main__':
    main()
