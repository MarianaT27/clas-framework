"""
dt_fit_check.py : the Gaussian slice fits of dt_figs.py drawn on the Delta-t distributions.

Usage (from the clas-framework folder, after dt_figs.py):
    python3 ib_check/pion/dt_fit_check.py /volatile/clas12/mtenorio/pi0/ib_check/pion

Reads   <dir>/dt/dt_slice_fits.csv (mu, sigma of each slice fit, and the parametrization values)
        <dir>/root/pion_ana_<T>_<POL>.root, PionAnalysis/Pi{Plus,Minus}/delta_t_vs_p_diag
Writes  <dir>/dt/dt_fit_check_<POL>_<species>.png/.pdf
        rows = 8 momentum slices, columns = LD2, CxC, CuSn
Each panel: Delta-t of the slice (histogram), the fitted Gaussian (mu and sigma from the CSV; only the
height is recomputed, by least squares in the fit window), the fit window mu_param +- 2.5 sigma_param
(shaded), and mu_param +- 3 sigma_param of the common-note parametrization (dashed).
"""
import csv
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import uproot

DIR = Path(sys.argv[1])
OUT = DIR / "dt"
TARGETS = ["LD2", "CxC", "CuSn"]
COLOR = {"LD2": "black", "CxC": "#1f77b4", "CuSn": "#9467bd"}
SPECIES = {"PiPlus": r"$\pi^+$", "PiMinus": r"$\pi^-$"}
SLICES = [0.6, 1.2, 2.1, 3.0, 4.2, 5.4, 6.6, 7.5]  # lower edges, matching the 0.3 GeV slices of dt_figs.py
FIT_NSIG, CUT_NSIG = 2.5, 3.0

fits = {}
for r in csv.DictReader(open(OUT / "dt_slice_fits.csv")):
    fits[(r["polarity"], r["species"], r["target"], round(float(r["p_lo"]), 3))] = r


def find(f, suffix):
    for k in f.keys(recursive=True, cycle=False):
        if k.endswith(suffix):
            return f[k]
    return None


hists = {}
for pol in ["IB", "OB"]:
    for t in TARGETS:
        fn = DIR / "root" / f"pion_ana_{t}_{pol}.root"
        if fn.exists():
            f = uproot.open(fn)
            for sp in SPECIES:
                h = find(f, f"PionAnalysis/{sp}/delta_t_vs_p_diag")
                if h is not None:
                    hists[(pol, sp, t)] = (h.values(), h.axis(0).edges(), h.axis(1).edges())

for pol in ["IB", "OB"]:
    for sp in SPECIES:
        fig, axs = plt.subplots(len(SLICES), len(TARGETS), figsize=(4.2 * len(TARGETS), 2.3 * len(SLICES)), squeeze=False)
        for j, t in enumerate(TARGETS):
            for i, lo in enumerate(SLICES):
                ax = axs[i, j]
                r = fits.get((pol, sp, t, round(lo, 3)))
                d = hists.get((pol, sp, t))
                if r is None or d is None:
                    ax.text(0.5, 0.5, "no fit", ha="center", transform=ax.transAxes)
                    continue
                v, xe, ye = d
                hi = float(r["p_hi"])
                sel = (xe[:-1] >= lo - 1e-9) & (xe[1:] <= hi + 1e-9)
                y = v[sel].sum(axis=0)
                yc = 0.5 * (ye[1:] + ye[:-1])
                mu, sg = float(r["mu_fit"]), float(r["sigma_fit"])
                mp, sp_ = float(r["mu_param"]), float(r["sigma_param"])
                win = np.abs(yc - mp) < FIT_NSIG * sp_
                g = np.exp(-0.5 * ((yc - mu) / sg) ** 2)
                amp = np.sum(y[win] * g[win]) / np.sum(g[win] ** 2)
                ax.stairs(y, ye, color=COLOR[t], lw=1)
                xx = np.linspace(-1, 1, 400)
                ax.plot(xx, amp * np.exp(-0.5 * ((xx - mu) / sg) ** 2), color="red", lw=1.2,
                        label=rf"fit $\mu$={mu:.3f}, $\sigma$={sg:.3f}")
                ax.axvspan(mp - FIT_NSIG * sp_, mp + FIT_NSIG * sp_, color="gray", alpha=0.15, lw=0)
                for s in (-1, 1):
                    ax.axvline(mp + s * CUT_NSIG * sp_, color="gray", ls="--", lw=0.9)
                ax.set_xlim(-1, 1)
                ax.set_title(rf"{t}, $p\in$[{lo:.1f}, {hi:.1f}) GeV", fontsize=9)
                ax.legend(fontsize=6.5, loc="upper right")
                ax.tick_params(labelsize=7)
                if i == len(SLICES) - 1:
                    ax.set_xlabel(r"$\Delta t$ [ns]")
        tab = 6 if pol == "OB" else 7
        fig.suptitle(f"{pol}, {SPECIES[sp]}: Gaussian slice fits (red), fit window (shaded), "
                     f"$\\mu\\pm3\\sigma$ of common note Table {tab} (dashed)", fontsize=11, y=1.005)
        fig.tight_layout()
        for ext in ("png", "pdf"):
            fig.savefig(OUT / f"dt_fit_check_{pol}_{sp}.{ext}", dpi=130, bbox_inches="tight")
        plt.close(fig)
        print("wrote", OUT / f"dt_fit_check_{pol}_{sp}.png")
