#!/usr/bin/env bash
#
# compare_electron_benchmark.sh
#
# Head-to-head benchmark: the chain-only `benchmark_electron` vs the lean
# framework `electron_selection` (ElectronSelectionAlg only), on the same
# HIPO file, across a thread sweep. Prints a side-by-side table (wall time,
# events/sec, peak RSS, speedup) and dumps both cutflows for parity checking.
#
# Both binaries must be built first (ninja -C build). macOS /usr/bin/time -l
# is used for wall time + peak RSS.
#
# Usage:
#   tools/compare_electron_benchmark.sh [HIPO_FILE] [THREADS...]
#
# Defaults:
#   HIPO_FILE = /Users/mathieuouillon/Documents/tmp/hipo-converter/rec0.hipo
#   THREADS   = 1 4 8
#   PERCENT   = 100   (override with PERCENT=NN env var)

set -uo pipefail   # NB: no -e; a single timed run failing must not abort the sweep

# ROOT (linked into the framework binary) initializes CoreFoundation on macOS;
# when launched through nested fork layers the CF fork-guard can abort. These
# make child processes fork-safe so the sweep runs cleanly.
export OBJC_DISABLE_INITIALIZE_FORK_SAFETY=YES
unset DISPLAY 2>/dev/null || true

# ---- locate repo + binaries -------------------------------------------------
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"

STANDALONE="$BUILD_DIR/tools/benchmark_electron"
FRAMEWORK="$BUILD_DIR/analyses/electron_selection"
TEMPLATE_TOML="$REPO_ROOT/config/electron_selection.toml"

HIPO_FILE="${1:-/Users/mathieuouillon/Documents/tmp/hipo-converter/rec0.hipo}"
shift || true
THREADS=("$@")
[ ${#THREADS[@]} -eq 0 ] && THREADS=(1 4 8)
PERCENT="${PERCENT:-100}"

for f in "$STANDALONE" "$FRAMEWORK" "$TEMPLATE_TOML" "$HIPO_FILE"; do
    if [ ! -e "$f" ]; then
        echo "ERROR: missing required path: $f" >&2
        [ "$f" = "$STANDALONE" ] || [ "$f" = "$FRAMEWORK" ] && \
            echo "       (build first: ninja -C \"$BUILD_DIR\")" >&2
        exit 1
    fi
done

TIME_BIN="/usr/bin/time"   # macOS BSD time with -l (peak RSS)
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# ---- helpers ----------------------------------------------------------------
# Parse "real" seconds from a /usr/bin/time -l stderr capture.
parse_real() { grep -E '[[:space:]]real' "$1" | tail -1 | awk '{print $1}'; }
# Parse peak RSS (bytes) -> MB.
parse_rss_mb() {
    local b
    b="$(grep -E 'maximum resident set size' "$1" | tail -1 | awk '{print $1}')"
    [ -n "${b:-}" ] && awk -v x="$b" 'BEGIN{printf "%.0f", x/1048576}' || echo "?"
}

make_toml() {  # $1=threads -> echoes temp toml path
    local n="$1" out="$WORK/sel_${n}.toml"
    sed -E \
        -e "s|^input[[:space:]]*=.*|input       = \"$HIPO_FILE\"|" \
        -e "s|^num_threads[[:space:]]*=.*|num_threads = $n|" \
        -e "s|^percentage[[:space:]]*=.*|percentage  = $PERCENT|" \
        "$TEMPLATE_TOML" > "$out"
    echo "$out"
}

echo "=============================================================================="
echo " Electron-selection benchmark: chain-only vs framework"
echo "   file       : $HIPO_FILE"
echo "   threads    : ${THREADS[*]}"
echo "   percentage : $PERCENT"
echo "=============================================================================="
printf "%-8s | %-28s | %-28s | %-8s\n" "threads" "chain-only" "framework" "speedup"
printf "%-8s | %-12s %-15s | %-12s %-15s | %-8s\n" "" "wall(s)" "ev/s  RSS(MB)" "wall(s)" "ev/s  RSS(MB)" "(fw/co)"
echo "------------------------------------------------------------------------------"

for N in "${THREADS[@]}"; do
    toml="$(make_toml "$N")"

    # Warm the page cache (discard this run).
    "$STANDALONE" "$HIPO_FILE" --threads "$N" --percentage "$PERCENT" >/dev/null 2>&1 || true

    # --- chain-only (standalone) ---
    co_log="$WORK/co_${N}.out"; co_time="$WORK/co_${N}.time"
    "$TIME_BIN" -l "$STANDALONE" "$HIPO_FILE" --threads "$N" --percentage "$PERCENT" \
        >"$co_log" 2>"$co_time"
    co_wall="$(parse_real "$co_time")"
    co_rss="$(parse_rss_mb "$co_time")"
    co_events="$(grep -E 'events processed' "$co_log" | tail -1 | awk '{print $NF}')"
    co_evps="$(awk -v e="${co_events:-0}" -v w="${co_wall:-0}" \
        'BEGIN{ if (w>0) printf "%.0f", e/w; else printf "0" }')"

    # --- framework (electron_selection) ---
    fw_log="$WORK/fw_${N}.out"; fw_time="$WORK/fw_${N}.time"
    "$TIME_BIN" -l "$FRAMEWORK" --config "$toml" >"$fw_log" 2>"$fw_time" || true
    fw_wall="$(parse_real "$fw_time")"
    fw_rss="$(parse_rss_mb "$fw_time")"
    fw_evps="$(awk -v e="${co_events:-0}" -v w="${fw_wall:-0}" \
        'BEGIN{ if (w>0) printf "%.0f", e/w; else printf "0" }')"

    speedup="$(awk -v f="${fw_wall:-0}" -v c="${co_wall:-0}" \
        'BEGIN{ if (c>0) printf "%.2fx", f/c; else printf "?" }')"

    printf "%-8s | %-12s %-7s %-7s | %-12s %-7s %-7s | %-8s\n" \
        "$N" "$co_wall" "$co_evps" "$co_rss" "$fw_wall" "$fw_evps" "$fw_rss" "$speedup"
done

echo "------------------------------------------------------------------------------"
echo
echo "Cutflow parity check (threads=${THREADS[0]}, percentage=$PERCENT):"
echo "  events processed (chain-only): ${co_events:-?}"
echo
echo "--- chain-only cutflow ---"
sed -n '/stage/,/====/p' "$WORK/co_${THREADS[0]}.out" || true
echo
echo "--- framework cutflow (ElectronSelection) ---"
grep -iE 'cut|electron|events|pass|%' "$WORK/fw_${THREADS[0]}.out" | grep -iv 'INFO.*Initial' || \
    echo "  (see full framework log: $WORK/fw_${THREADS[0]}.out — but it is deleted on exit;"
echo
echo "Note: the framework cutflow is printed by ElectronSelectionAlg at finalize."
echo "      Compare its per-stage counts against the chain-only block above; they"
echo "      must match exactly at percentage=100. Stage labels differ slightly"
echo "      (e.g. 'Momentum > 0.8 GeV' is the framework's stale label for p>2.0)."
