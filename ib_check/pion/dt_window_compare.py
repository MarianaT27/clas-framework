"""
dt_window_compare.py : pions kept by the Delta-t cut mu(p) +- 3 sigma(p), this work's Eq. 11/12 parameters vs the
common note (Table 6 OB, Table 7 IB).

Usage (from the clas-framework folder, after dt_figs.py):
    python3 ib_check/pion/dt_window_compare.py /volatile/clas12/mtenorio/pi0/ib_check/pion

Reads   <dir>/dt/dt_params.csv                    (written by dt_figs.py, both parameter sets)
        <dir>/root/pion_ana_<T>_<POL>.root        PionAnalysis/Pi{Plus,Minus}/delta_t_vs_p_diag
Counts each (p, Delta-t) histogram bin by its centre, for 0.3 <= p < 8.0 GeV (the range of the fits).
For CuSn, this work's single fit is compared with the Cu row and with the Sn row of the note.
Columns: N note, N ours = pions inside each window; gained = inside ours only; lost = inside the note's only;
         net = (N ours - N note) / N note.
"""
import csv
import sys
from pathlib import Path

import numpy as np
import uproot

DIR = Path(sys.argv[1])
P_MIN, P_MAX, NSIG = 0.3, 8.0, 3.0
PAR = ["a_mu", "b_mu", "c_mu", "sigma0", "k", "c_sigma"]


def window(p, a, b, c, s0, k, cs):
    mu = a * p**2 + b * p + c
    sg = np.sqrt(s0**2 + (k / p) ** 2) + cs * p
    return mu - NSIG * sg, mu + NSIG * sg


def find(f, suffix):
    for key in f.keys(recursive=True, cycle=False):
        if key.endswith(suffix):
            return f[key]
    return None


rows = list(csv.DictReader(open(DIR / "dt" / "dt_params.csv")))
print(f"{'pol':3} {'species':8} {'target':11} {'N note':>11} {'N ours':>11} {'gained':>9} {'lost':>9} {'net':>7}")
for r in rows:
    if r["source"] != "this work":
        continue
    pol, sp, t = r["polarity"], r["species"], r["target"]
    f = uproot.open(DIR / "root" / f"pion_ana_{t}_{pol}.root")
    h = find(f, f"PionAnalysis/{sp}/delta_t_vs_p_diag")
    v, xe, ye = h.values(), h.axis(0).edges(), h.axis(1).edges()
    pc, yc = 0.5 * (xe[1:] + xe[:-1]), 0.5 * (ye[1:] + ye[:-1])
    sel = (pc >= P_MIN) & (pc < P_MAX)
    P, Y = np.meshgrid(pc[sel], yc, indexing="ij")
    W = v[sel]
    lo, hi = window(P, *[float(r[n]) for n in PAR])
    in_ours = (Y >= lo) & (Y <= hi)
    for n in rows:
        if n["source"] == "this work" or (n["polarity"], n["species"], n["target"]) != (pol, sp, t):
            continue
        lo_n, hi_n = window(P, *[float(n[x]) for x in PAR])
        in_note = (Y >= lo_n) & (Y <= hi_n)
        n_note, n_ours = W[in_note].sum(), W[in_ours].sum()
        gained, lost = W[in_ours & ~in_note].sum(), W[in_note & ~in_ours].sum()
        foil = n["source"].split("(")[-1].rstrip(")")
        tag = t if foil == t else f"{t} vs {foil}"
        print(f"{pol:3} {sp:8} {tag:11} {n_note:11.0f} {n_ours:11.0f} {gained:9.0f} {lost:9.0f} "
              f"{100 * (n_ours - n_note) / n_note:+6.2f}%")
