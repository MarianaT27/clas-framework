#!/usr/bin/env python3
"""
Drive `local_batch` for the pi0 multiplicity analysis across all targets.

For each target it runs, SEQUENTIALLY (one target's batch finishes before the
next starts; local_batch itself fans out --concurrent slices):

    ./build/tools/local_batch \
        --exe ./build/analyses/pi0_multiplicity \
        --config config/pi0_multiplicity_<target>.toml \
        --slices <N> --concurrent <M> \
        --output-dir <output-base>/pi0_<target>_<suffix> [--automerge]

The defaults reproduce the canonical Sn / Cu / CxC / LD2 run
(output-base=/volatile/clas12/ouillon, suffix=2, slices=100, concurrent=50).

Run it from the repository root (the ./build and config paths are relative).

Examples:
    python scripts/run_pi0_batch.py                          # all 4 targets, suffix 2
    python scripts/run_pi0_batch.py --suffix 3 --slices 200 --concurrent 60
    python scripts/run_pi0_batch.py --targets Sn LD2 --dry-run
    python scripts/run_pi0_batch.py --output-base /work/clas12/me/pi0
    python scripts/run_pi0_batch.py -- --profile            # pass extra flags through
"""

from __future__ import annotations

import argparse
import shlex
import subprocess
import sys
import time
from pathlib import Path

DEFAULT_TARGETS = ["Sn", "Cu", "CxC", "LD2"]


def build_command(args, target: str):
    config = args.config_pattern.format(target=target)
    outdir = f"{args.output_base.rstrip('/')}/pi0_{target}_{args.suffix}"
    cmd = [
        args.local_batch,
        "--exe", args.exe,
        "--config", config,
        "--slices", str(args.slices),
        "--concurrent", str(args.concurrent),
        "--output-dir", outdir,
    ]
    if args.automerge:
        cmd.append("--automerge")
    cmd += args.extra
    return cmd, config, outdir


def main() -> None:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--output-base", default="/volatile/clas12/ouillon",
                    help="parent dir; per-target output = <base>/pi0_<target>_<suffix> "
                         "(default: %(default)s)")
    ap.add_argument("--suffix", default="2",
                    help="tag appended to each target's output folder (default: %(default)s)")
    ap.add_argument("--slices", type=int, default=100,
                    help="local_batch --slices (default: %(default)s)")
    ap.add_argument("--concurrent", type=int, default=50,
                    help="local_batch --concurrent (default: %(default)s)")
    ap.add_argument("--targets", nargs="+", default=DEFAULT_TARGETS, metavar="T",
                    help="targets to run (default: %(default)s)")
    ap.add_argument("--exe", default="./build/analyses/pi0_multiplicity",
                    help="analysis executable (default: %(default)s)")
    ap.add_argument("--local-batch", default="./build/tools/local_batch",
                    help="local_batch tool (default: %(default)s)")
    ap.add_argument("--config-pattern", default="config/pi0_multiplicity_{target}.toml",
                    help="config path with {target} substituted (default: %(default)s)")
    ap.add_argument("--no-automerge", dest="automerge", action="store_false",
                    help="do not pass --automerge to local_batch")
    ap.add_argument("--stop-on-error", action="store_true",
                    help="abort the remaining targets if one fails")
    ap.add_argument("--dry-run", action="store_true",
                    help="print the commands without running them")
    ap.add_argument("extra", nargs="*", metavar="-- EXTRA",
                    help="extra args appended to every local_batch call (put them after a '--')")
    ap.set_defaults(automerge=True)
    args = ap.parse_args()

    # Pre-flight: tools present (skipped in dry-run so the plan previews anywhere).
    if not args.dry_run:
        for path, what in [(args.local_batch, "local_batch tool"), (args.exe, "analysis exe")]:
            if not Path(path).exists():
                sys.exit(f"error: {what} not found at '{path}'. Run from the repo root "
                         "(where ./build lives), or pass the correct path.")

    plans = []
    for target in args.targets:
        cmd, config, outdir = build_command(args, target)
        plans.append((target, cmd, config, outdir, not Path(config).exists()))

    print(f"Targets: {', '.join(args.targets)}   "
          f"slices={args.slices} concurrent={args.concurrent} automerge={args.automerge}")
    for target, _cmd, _config, outdir, missing in plans:
        print(f"  {target:5s} -> {outdir}{'   [config MISSING]' if missing else ''}")
    print()

    if args.dry_run:
        for _t, cmd, *_ in plans:
            print(" ".join(shlex.quote(c) for c in cmd))
        return

    results = []
    for target, cmd, config, _outdir, missing in plans:
        if missing:
            print(f"=== SKIP {target}: config '{config}' not found ===", flush=True)
            results.append((target, "skipped", 0.0))
            if args.stop_on_error:
                sys.exit(f"error: config for {target} missing and --stop-on-error set.")
            continue
        bar = "=" * 70
        print(f"\n{bar}\n=== {target}: {' '.join(shlex.quote(c) for c in cmd)}\n{bar}", flush=True)
        t0 = time.monotonic()
        rc = subprocess.run(cmd).returncode
        dt = time.monotonic() - t0
        status = "ok" if rc == 0 else f"FAILED (exit {rc})"
        results.append((target, status, dt))
        print(f"=== {target}: {status} in {dt / 60:.1f} min ===", flush=True)
        if rc != 0 and args.stop_on_error:
            break

    print(f"\n{'=' * 70}\nSummary\n{'=' * 70}")
    nfail = sum(1 for _t, s, _dt in results if s.startswith("FAILED"))
    for target, status, dt in results:
        print(f"  {target:5s}  {status:18s}  {dt / 60:5.1f} min")
    sys.exit(1 if nfail else 0)


if __name__ == "__main__":
    main()
