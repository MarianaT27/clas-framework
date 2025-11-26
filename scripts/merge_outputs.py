#!/usr/bin/env python3
"""
Merge ROOT output files from batch jobs
"""

import argparse
import subprocess
import glob
import os
import sys
from pathlib import Path


def merge_root_files(input_files, output_file, verbose=False):
    """Merge ROOT files using hadd"""
    if not input_files:
        print("ERROR: No input files to merge", file=sys.stderr)
        return False

    print(f"Merging {len(input_files)} files into {output_file}")

    # Build hadd command
    cmd = ['hadd', '-f', output_file] + input_files

    if verbose:
        print(f"Command: {' '.join(cmd)}")

    try:
        result = subprocess.run(
            cmd,
            check=True,
            capture_output=not verbose,
            text=True
        )

        if verbose and result.stdout:
            print(result.stdout)

        print(f"Successfully merged files into: {output_file}")
        return True

    except subprocess.CalledProcessError as e:
        print(f"ERROR: Failed to merge files", file=sys.stderr)
        if e.stderr:
            print(e.stderr, file=sys.stderr)
        return False


def find_output_files(output_dir, pattern="*.root"):
    """Find all ROOT files in output directory"""
    files = []

    # Search in subdirectories
    search_pattern = os.path.join(output_dir, "**", pattern)
    files = glob.glob(search_pattern, recursive=True)

    return sorted(files)


def merge_by_type(output_dir, merged_dir, verbose=False):
    """Merge files by type (e.g., all histogram files, all ntuple files)"""
    Path(merged_dir).mkdir(parents=True, exist_ok=True)

    # Common file patterns
    patterns = {
        'histograms': '*histogram*.root',
        'ntuples': '*ntuple*.root',
        'analysis': '*analysis*.root',
        'all': '*.root'
    }

    merged_files = []

    for name, pattern in patterns.items():
        files = find_output_files(output_dir, pattern)

        if not files:
            print(f"No files found matching pattern: {pattern}")
            continue

        output_file = os.path.join(merged_dir, f"merged_{name}.root")

        if merge_root_files(files, output_file, verbose):
            merged_files.append(output_file)
            print(f"  → {output_file}")

    return merged_files


def main():
    parser = argparse.ArgumentParser(
        description='Merge ROOT output files from batch jobs'
    )
    parser.add_argument(
        '--input-dir',
        required=True,
        help='Directory containing job outputs'
    )
    parser.add_argument(
        '--output',
        default='merged_output.root',
        help='Output merged file name'
    )
    parser.add_argument(
        '--pattern',
        default='*.root',
        help='File pattern to match'
    )
    parser.add_argument(
        '--by-type',
        action='store_true',
        help='Merge files by type (histograms, ntuples, etc.)'
    )
    parser.add_argument(
        '--merged-dir',
        default='./merged',
        help='Directory for merged files (when using --by-type)'
    )
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Verbose output'
    )

    args = parser.parse_args()

    # Check if hadd is available
    try:
        subprocess.run(['hadd', '-h'], capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("ERROR: hadd command not found. Make sure ROOT is in your PATH.", file=sys.stderr)
        sys.exit(1)

    # Find files
    if args.by_type:
        merged_files = merge_by_type(args.input_dir, args.merged_dir, args.verbose)
        if merged_files:
            print(f"\nMerged {len(merged_files)} file types successfully")
    else:
        input_files = find_output_files(args.input_dir, args.pattern)

        if not input_files:
            print(f"ERROR: No files found matching pattern: {args.pattern}", file=sys.stderr)
            sys.exit(1)

        success = merge_root_files(input_files, args.output, args.verbose)
        sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
