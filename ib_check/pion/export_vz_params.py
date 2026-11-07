"""
export_vz_params.py : write the Vz correction npz files (electron and pion) into the text format read by
VzCorrector::load_from_file. Standalone version of export_cpp_params.py (python-electron-vz): reads the npz
files with numpy only (no ROOT / mplhep / vz_correction import). Same layout and number format.

Usage (from the clas-framework folder):
    python3 ib_check/pion/export_vz_params.py <npz_dir> <out_file> [--electron-only]

<npz_dir>  e.g. /work/clas12/ouillon/archive/python-electron-vz/output/npz
Blocks written: cusn_outbending, cusn_inbending, cxc_outbending, cxc_inbending (electron, same order as
data/Vz/vz_corrector_params.txt), then <variant>_pim and <variant>_pip for each (pion), unless --electron-only.
npz names follow vz_correction.VARIANTS and _make_pion_variant (base name minus "_params.npz" + "_<tag>_params.npz").
"""
import sys
from pathlib import Path

import numpy as np

N_PHI, N_SECTORS, N_POLY = 6, 6, 4
ELECTRON = [
    ("cusn_outbending", "vz_correction_params.npz"),
    ("cusn_inbending", "vz_correction_inbending_params.npz"),
    ("cxc_outbending", "vz_correction_cxc_outbending_params.npz"),
    ("cxc_inbending", "vz_correction_cxc_inbending_params.npz"),
]
PION_TAGS = ["pim", "pip"]


def peak_names(d):
    """Same logic as vz_correction.load_params: lo = less-negative peak."""
    if "peak_names" in d.files:
        lo, hi = (str(x) for x in np.atleast_1d(d["peak_names"]))
    elif "mu0_sn" in d.files:
        lo, hi = "Sn", "Cu"
    elif "mu0_c1" in d.files:
        lo, hi = "C1", "C2"
    else:
        sys.exit(f"unrecognized npz schema: {sorted(d.files)}")
    return lo.lower(), hi.lower()


def write_variant(f, key, path):
    d = np.load(path)
    lo, hi = peak_names(d)
    p_edges = d["p_edges"]
    n_p = len(p_edges) - 1
    f.write(f"{key} {float(d[f'mu0_{lo}']):.10g} {float(d[f'sigma0_{lo}']):.10g} "
            f"{float(d[f'mu0_{hi}']):.10g} {float(d[f'sigma0_{hi}']):.10g} "
            f"{float(p_edges[0]):.10g} {float(p_edges[1] - p_edges[0]):.10g} {n_p}\n")
    arrays = [d[f"poly_mu_{lo}_th"], d[f"poly_mu_{hi}_th"], d[f"poly_sig_{lo}_th"], d[f"poly_sig_{hi}_th"]]
    dom = d["theta_domains"]
    for s in range(N_SECTORS):
        for ip in range(n_p):
            for iphi in range(N_PHI):
                coefs = [float(c) for a in arrays for c in a[s, ip, iphi]]
                f.write(f"{s + 1} {ip} {iphi}  " + " ".join(f"{c:.10g}" for c in coefs)
                        + f"  {float(dom[s, ip, iphi][0]):.10g} {float(dom[s, ip, iphi][1]):.10g}\n")
    return n_p


npz_dir, out = Path(sys.argv[1]), Path(sys.argv[2])
blocks = list(ELECTRON)
if "--electron-only" not in sys.argv:
    for key, fn in ELECTRON:
        base = fn[:-len("_params.npz")]
        blocks += [(f"{key}_{t}", f"{base}_{t}_params.npz") for t in PION_TAGS]
missing = [fn for _, fn in blocks if not (npz_dir / fn).exists()]
if missing:
    sys.exit(f"missing npz in {npz_dir}: {missing}")

with open(out, "w") as f:
    f.write("# Multi-variant vz_corrector params\n")
    f.write("# Schema (whitespace-separated; '#' = comment):\n"
            "#   line 1: n_variants n_phi n_sectors n_poly\n"
            "#   per variant block:\n"
            "#     header: variant_key mu0_lo sigma0_lo mu0_hi sigma0_hi p_lo p_binw n_p\n"
            "#     then n_sectors*n_p*n_phi rows:\n"
            "#       sector ip iphi  c_mu_lo[4]  c_mu_hi[4]  c_sig_lo[4]  c_sig_hi[4]  th_min th_max\n")
    f.write(f"{len(blocks)} {N_PHI} {N_SECTORS} {N_POLY}\n")
    for key, fn in blocks:
        n_p = write_variant(f, key, npz_dir / fn)
        print(f"  {key:22s} <- {fn}  (n_p = {n_p})")
print(f"wrote {out} with {len(blocks)} variants")
