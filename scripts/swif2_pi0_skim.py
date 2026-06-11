#!/usr/bin/env python3
"""
Generate (and optionally submit) a SWIF2 workflow that runs the pi0 skim over a
list of /mss HIPO files on the JLab batch farm, copying each slim skim back to
/volatile, organized one subdirectory per run.

For each chunk of `--files-per-job` input files FROM THE SAME RUN it emits one
`swif2 add-job` that:
  * stages the /mss files onto the worker node as input_0.hipo, input_1.hipo, ...
    (SWIF2 -input routes /mss paths through the tape system automatically),
  * runs `pi0_multiplicity_skim --input <staged,...> --hipo-output result.hipo`
    via a shared `wrapper.sh` (module loads + the skim; the runner's --input takes
    the comma-separated list the wrapper builds by globbing the staged files),
  * copies `result.hipo` back to <output-dir>/<run>/<stem>.hipo and the log to
    <output-dir>/logs/ (SWIF2 -output).

The run number is parsed from each filename (`rec_clas_<run>.evio...`, falling
back to a numeric parent directory). The generated script creates every
<output-dir>/<run>/ subdirectory before submitting.

To decouple a queued workflow from ongoing development, the program is FROZEN at
generation time: the executable, its user-built shared libraries (the binary
links libclas12-framework/algorithms/hipo as .so), and the config are copied into
<scripts-dir>/<workflow>_prog, and the wrapper runs that copy with
LD_LIBRARY_PATH pointed there. So you can rebuild / switch branches while jobs sit
in the queue and they still run the program as it was when you submitted. Keep
that dir until the workflow finishes. Use --no-snapshot to reference the live
build instead.

Inputs come from a plain text list (one path per line, '#' comments ignored) such
as `ld2_files.dat`. This complements the general `./build/tools/batch` tool,
which targets .root analyses; the skim needs a file list and the .hipo output.

Run from the repository root ON the JLab login node (ifarm). Dry-run by default:
it writes `batch_scripts/<workflow>.swif2.sh` + `wrapper.sh` and prints a summary;
add --submit to actually run it.

Examples:
    python scripts/swif2_pi0_skim.py ld2_files.dat --module gcc/13.2.0 root/6.36.04
    python scripts/swif2_pi0_skim.py ld2_files.dat --files-per-job 10 --disk 60G --time 8:00:00
    python scripts/swif2_pi0_skim.py ld2_files.dat --debug --module ...      # 10 jobs, priority
    python scripts/swif2_pi0_skim.py ld2_files.dat --module ... --submit
"""

from __future__ import annotations

import argparse
import re
import shlex
import shutil
import stat
import subprocess
import sys
from collections import OrderedDict
from pathlib import Path

REPO = Path.cwd()
RUN_RE = re.compile(r"(?:rec_clas|clas)_(\d+)")


def read_file_list(path: Path) -> list[str]:
    out = []
    for line in path.read_text().splitlines():
        s = line.strip()
        if s and not s.startswith("#"):
            out.append(s)
    return out


def stem_of(mss_path: str) -> str:
    name = Path(mss_path).name
    for ext in (".hipo", ".evio"):
        if name.endswith(ext):
            name = name[: -len(ext)]
    return name


def run_of(mss_path: str) -> str:
    """Run number for the per-run output subdirectory: from the filename, else a
    numeric parent directory, else 'misc'."""
    m = RUN_RE.search(Path(mss_path).name)
    if m:
        return m.group(1)
    parent = Path(mss_path).parent.name
    return parent if parent.isdigit() else "misc"


def snapshot_program(exe: str, config: str, build_dir: str, snap_dir: Path):
    """Freeze the program so the workflow is immune to later rebuilds: copy the
    executable, the config, and every user-built shared library from build_dir
    into snap_dir. The wrapper then runs the copy with LD_LIBRARY_PATH=snap_dir
    (the binary links libclas12-framework/algorithms/hipo as shared objects, so
    the libs must be frozen too). Returns (exe_copy, config_copy, snap_dir, nlibs)
    as absolute paths."""
    snap = snap_dir.resolve()
    snap.mkdir(parents=True, exist_ok=True)
    exe_dst = snap / Path(exe).name
    shutil.copy2(exe, exe_dst)
    exe_dst.chmod(exe_dst.stat().st_mode | stat.S_IEXEC)
    cfg_dst = snap / Path(config).name
    shutil.copy2(config, cfg_dst)
    nlibs = 0
    bd = Path(build_dir)
    seen = set()
    for pat in ("*.so", "*.so.*", "*.dylib"):
        for lib in bd.rglob(pat):
            # Skip meson's private build dirs (e.g. libclas12.so.p) and any other
            # non-regular-file match; copy each library basename once.
            if not lib.is_file() or lib.name in seen:
                continue
            seen.add(lib.name)
            shutil.copy2(lib, snap / lib.name)
            nlibs += 1
    return str(exe_dst), str(cfg_dst), str(snap), nlibs


def wrapper_text(exe: str, config: str, modules: list[str], setup: str,
                 lib_dir: str | None = None) -> str:
    lines = ["#!/usr/bin/env bash", "set -uo pipefail", ""]
    lines += [f"module load {m}" for m in modules]
    if setup:
        lines.append(setup)
    if lib_dir:
        lines.append(f"export LD_LIBRARY_PATH={shlex.quote(lib_dir)}:${{LD_LIBRARY_PATH:-}}")
    lines += [
        "",
        "# SWIF2 stages this job's files as input_0.hipo, input_1.hipo, ...;",
        "# gather them into the comma-separated list the runner's --input accepts.",
        'inputs=$(ls input_*.hipo 2>/dev/null | sort | paste -sd, -)',
        'if [ -z "$inputs" ]; then',
        '    echo "ERROR: no input_*.hipo in $(pwd) -- run this inside a SWIF2 job (it stages the inputs)." >&2',
        '    exit 1',
        'fi',
        f"{shlex.quote(exe)} --config {shlex.quote(config)} \\",
        '    --input "$inputs" --output result.root --hipo-output result.hipo \\',
        "    2>&1 | tee log.txt",
        "exit ${PIPESTATUS[0]}",
        "",
    ]
    return "\n".join(lines)


def add_job_block(args, idx: int, run: str, chunk: list[str], wrapper_abs: str) -> str:
    job = f"{args.workflow}_job_{idx}"
    out_hipo = f"{args.output_dir.rstrip('/')}/{run}/{stem_of(chunk[0])}.hipo"
    out_log = f"{args.output_dir.rstrip('/')}/logs/{job}.log"
    parts = [
        "swif2 add-job",
        f"-workflow {shlex.quote(args.workflow)}",
        f"-name {shlex.quote(job)}",
        f"-account {shlex.quote(args.account)}",
        f"-partition {shlex.quote(args.partition)}",
        f"-cores {args.cores}",
        f"-ram {shlex.quote(args.ram)}",
        f"-disk {shlex.quote(args.disk)}",
        f"-time {shlex.quote(args.time)}",
    ]
    if args.constraint:
        parts.append(f"-constraint {shlex.quote(args.constraint)}")
    for j, mss in enumerate(chunk):
        parts.append(f"-input input_{j}.hipo {shlex.quote(mss)}")
    # Run the wrapper by absolute path rather than SWIF2 -input-staging it: the
    # compute node reads /work (/w) directly -- that's already how it reaches the
    # frozen exe/libs/config -- and SWIF2 won't tape-stage a CephFS path like it
    # does /mss. The wrapper's CWD is still the job dir, so it sees the staged
    # input_*.hipo and writes result.hipo / log.txt there for the -output copies.
    parts += [
        f"-output result.hipo {shlex.quote(out_hipo)}",
        f"-output log.txt {shlex.quote(out_log)}",
        f"bash {shlex.quote(wrapper_abs)}",
    ]
    return " \\\n    ".join(parts)


def main() -> None:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("file_list", help="text file, one /mss HIPO path per line (e.g. ld2_files.dat)")
    ap.add_argument("--workflow", default="pi0_skim_ld2", help="SWIF2 workflow name (default: %(default)s)")
    ap.add_argument("-o", "--output-dir", default="/volatile/clas12/ouillon/pi0_skim_LD2",
                    help="output root; skims land in <output-dir>/<run>/<stem>.hipo (default: %(default)s)")
    ap.add_argument("--files-per-job", type=int, default=1,
                    help="input files chained per job, within one run (default: %(default)s)")
    ap.add_argument("--debug", action="store_true",
                    help="quick test: 10 jobs, 1 file/job, partition 'priority'")
    ap.add_argument("--exe", default=str(REPO / "build/analyses/pi0_multiplicity_skim"),
                    help="skim executable, absolute (default: %(default)s)")
    ap.add_argument("--config", default=str(REPO / "config/pi0_multiplicity_skim.toml"),
                    help="skim config TOML, absolute (default: %(default)s)")
    ap.add_argument("--no-snapshot", action="store_true",
                    help="reference the live exe/config instead of freezing a copy (NOT recommended: a "
                         "rebuild would change what queued jobs run)")
    ap.add_argument("--snapshot-dir", default="",
                    help="where to freeze the program (default: <scripts-dir>/<workflow>_prog)")
    ap.add_argument("--build-dir", default="build",
                    help="build dir to copy shared libraries from for the snapshot (default: %(default)s)")
    ap.add_argument("--module", nargs="*", default=[], metavar="MOD",
                    help="module(s) to 'module load' in the job (e.g. gcc/13.2.0 root/6.36.04)")
    ap.add_argument("--setup", default="",
                    help="extra shell line to run after the module loads (e.g. 'source env.sh')")
    ap.add_argument("--scripts-dir", default="batch_scripts",
                    help="dir for the generated wrapper.sh + swif2 script (default: %(default)s)")
    ap.add_argument("--account", default="clas12")
    ap.add_argument("--partition", default="production")
    ap.add_argument("--cores", type=int, default=1)
    ap.add_argument("--ram", default="4G")
    ap.add_argument("--disk", default="20G")
    ap.add_argument("--time", default="4h", help="SWIF2 -time duration, e.g. 4h, 8h, 240m (default: %(default)s)")
    ap.add_argument("--constraint", default="", help="optional SWIF2 -constraint (e.g. el9)")
    ap.add_argument("--max-jobs", type=int, default=0, help="cap the number of jobs (0 = all; for testing)")
    ap.add_argument("--submit", action="store_true", help="run the generated swif2 script (default: just write it)")
    args = ap.parse_args()

    if args.debug:
        args.files_per_job = 1
        args.max_jobs = 10
        args.partition = "priority"

    flist = Path(args.file_list)
    if not flist.exists():
        sys.exit(f"error: file list '{flist}' not found.")
    files = read_file_list(flist)
    if not files:
        sys.exit(f"error: no input paths in '{flist}'.")

    # Group by run (preserving order), then chunk within each run so a job never
    # mixes runs and every output lands in its run's subdirectory.
    by_run: "OrderedDict[str, list[str]]" = OrderedDict()
    for f in files:
        by_run.setdefault(run_of(f), []).append(f)
    fpj = max(1, args.files_per_job)
    chunks = []  # (run, [files])
    for run, rfiles in by_run.items():
        for i in range(0, len(rfiles), fpj):
            chunks.append((run, rfiles[i:i + fpj]))
    if args.max_jobs > 0:
        chunks = chunks[: args.max_jobs]
    if not chunks:
        sys.exit("error: nothing to do.")
    runs = list(OrderedDict.fromkeys(run for run, _ in chunks))

    if not args.submit and not Path(args.exe).exists():
        print(f"note: skim exe '{args.exe}' not found locally (fine if you build before --submit).")

    scripts_dir = Path(args.scripts_dir)
    scripts_dir.mkdir(parents=True, exist_ok=True)
    wrapper_path = (scripts_dir / f"{args.workflow}_wrapper.sh").resolve()
    swif2_path = scripts_dir / f"{args.workflow}.swif2.sh"

    # Freeze the program so the queued workflow is immune to later rebuilds.
    exe, config, lib_dir, snap_note = args.exe, args.config, None, ""
    if not args.no_snapshot:
        if Path(args.exe).exists() and Path(args.config).exists():
            snap_dir = Path(args.snapshot_dir) if args.snapshot_dir else scripts_dir / f"{args.workflow}_prog"
            exe, config, lib_dir, nlibs = snapshot_program(args.exe, args.config, args.build_dir, snap_dir)
            snap_note = f"{lib_dir}  (exe + config + {nlibs} libs)"
        else:
            snap_note = "SKIPPED (exe/config not found locally; build first, or this is a remote dry-run)"

    wrapper_path.write_text(wrapper_text(exe, config, args.module, args.setup, lib_dir))
    wrapper_path.chmod(wrapper_path.stat().st_mode | stat.S_IEXEC)

    blocks = [
        f"# job {idx}: run {run}, {len(chunk)} file(s), first = {chunk[0]}\n"
        + add_job_block(args, idx, run, chunk, str(wrapper_path))
        for idx, (run, chunk) in enumerate(chunks)
    ]

    out_base = args.output_dir.rstrip("/")
    run_tokens = " ".join(shlex.quote(r) for r in runs)
    swif2 = [
        "#!/usr/bin/env bash",
        "set -euo pipefail",
        "",
        "# Create the output root, the log dir, and one subdirectory per run so",
        "# SWIF2's -output copies have a valid destination.",
        f'out_base={shlex.quote(out_base)}',
        'mkdir -p "$out_base/logs"',
        f'for r in {run_tokens}; do mkdir -p "$out_base/$r"; done',
        "",
        f"swif2 create -workflow {shlex.quote(args.workflow)}",
        "",
        "\n\n".join(blocks),
        "",
        f"swif2 run -workflow {shlex.quote(args.workflow)}",
        "",
    ]
    swif2_path.write_text("\n".join(swif2))
    swif2_path.chmod(swif2_path.stat().st_mode | stat.S_IEXEC)

    print(f"Workflow : {args.workflow}{'   [DEBUG]' if args.debug else ''}")
    print(f"Files    : {len(files)} across {len(by_run)} run(s)")
    print(f"Jobs     : {len(chunks)} ({fpj} file(s)/job, within a run) over {len(runs)} run dir(s)")
    print(f"Output   : {out_base}/<run>/<stem>.hipo  (+ logs/)")
    print(f"Resources: {args.cores} core, {args.ram} ram, {args.disk} disk, {args.time}, partition '{args.partition}'")
    print(f"Program  : {snap_note if snap_note else exe + ' (LIVE -- a rebuild will change queued jobs)'}")
    print(f"Wrapper  : {wrapper_path}")
    print(f"Script   : {swif2_path}")
    if lib_dir:
        print(f"           frozen at generation time -- keep this dir until the workflow finishes.")
    if not args.module:
        print("warning: no --module given; the job env may lack ROOT/compiler libs. "
              "Pass --module (e.g. --module gcc/13.2.0 root/6.36.04) or --setup.")
    if fpj > 1:
        print(f"note: with {fpj} files/job, size --disk for ~{fpj} staged reco files and --time accordingly.")

    if not args.submit:
        print(f"\nDry-run. Review {swif2_path}, then re-run with --submit (or `bash {swif2_path}`).")
        return

    print(f"\nSubmitting via {swif2_path} ...", flush=True)
    sys.exit(subprocess.run(["bash", str(swif2_path)]).returncode)


if __name__ == "__main__":
    main()
