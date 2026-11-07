"""
calo_figs.py : common-note Figs. 3.13 and 3.14 (PCAL fiducial cut, v > 9 cm and w > 9 cm), for IB or OB.

Usage:
    python3 calo_figs.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 calo_figs.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/calo

Reads:  <root_dir>/<ib|ob>_fig_{LD2,CxC,CuSn}.root   ElectronTracks/lv_vs_SF_S<s>, lw_vs_SF_S<s> (trigger electrons, before cuts)
        IB runs 18335, 18344, 18384; OB runs 18543, 18465, 18591
Writes: <out_dir>/fig3_13_sf_vs_vw.png/.pdf     profile of SF vs PCAL v (left) and w (right), 6 sectors, one row per target
        <out_dir>/fig3_14_sf_vs_v_targets.png/.pdf  profile of SF vs PCAL v, sector 1, LD2 / CxC / CuSn
Profile = mean SF in each v (or w) bin, error = RMS / sqrt(N); bins with fewer than 20 entries are not drawn.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import args, fname, label, COLORS, SECTOR_COLORS, save as _save

ROOT, POL, OUT = args()
TARGETS = ["LD2", "CxC", "CuSn"]
MIN_N, CUT = 20, 9.0

files = {t: uproot.open(fname(ROOT, POL, t)) for t in TARGETS}


def profile(f, coord, s):
    """Mean SF and its error in each bin of PCAL v or w."""
    h = f[f"Tracks/ElectronTracks/{coord}_vs_SF_S{s}"]
    v, ex, ey = h.values(), h.axis(0).edges(), h.axis(1).edges()
    xc, yc = 0.5 * (ex[1:] + ex[:-1]), 0.5 * (ey[1:] + ey[:-1])
    n = v.sum(axis=1)
    with np.errstate(invalid="ignore", divide="ignore"):
        mean = (v * yc).sum(axis=1) / n
        rms = np.sqrt((v * yc**2).sum(axis=1) / n - mean**2)
        err = rms / np.sqrt(n)
    ok = n >= MIN_N
    return xc[ok], mean[ok], err[ok]


def save(fig, name):
    _save(fig, OUT, name)


# ---- Fig. 3.13: SF vs v and w, 6 sectors, per target
fig, axs = plt.subplots(len(TARGETS), 2, figsize=(12, 4.2 * len(TARGETS)))
for r, t in enumerate(TARGETS):
    for c, (coord, name) in enumerate((("lv", "v"), ("lw", "w"))):
        ax = axs[r, c]
        for s in range(1, 7):
            x, m, e = profile(files[t], coord, s)
            ax.errorbar(x, m, yerr=e, fmt="o-", ms=2, lw=0.8, color=SECTOR_COLORS[s - 1], label=f"Sector {s}")
        ax.axvline(CUT, color="k", ls="--", label="9 cm cut")
        ax.set_xlim(0, 70)
        ax.set_ylim(0.12, 0.26)
        ax.set_xlabel(f"PCAL {name} [cm]")
        ax.set_ylabel("mean sampling fraction")
        ax.set_title(f"{label(POL, t)}: SF vs {name}, all sectors", fontsize=9)
        ax.grid(alpha=0.3)
        ax.legend(fontsize=7, ncol=2, loc="lower right", frameon=False)
save(fig, "fig3_13_sf_vs_vw")

# ---- Fig. 3.14: SF vs v, sector 1, targets
fig, ax = plt.subplots(figsize=(7, 4.5))
for t in TARGETS:
    x, m, e = profile(files[t], "lv", 1)
    ax.errorbar(x, m, yerr=e, fmt="o-", ms=2, lw=0.8, color=COLORS[t], label=label(POL, t))
ax.axvline(CUT, color="k", ls="--", label="9 cm cut")
ax.set_xlim(0, 70)
ax.set_ylim(0.12, 0.26)
ax.set_xlabel("PCAL v [cm]")
ax.set_ylabel("mean sampling fraction")
ax.set_title(f"Sector 1, {POL}: target comparison", fontsize=9)
ax.grid(alpha=0.3)
ax.legend(fontsize=8, loc="lower right", frameon=False)
save(fig, "fig3_14_sf_vs_v_targets")
