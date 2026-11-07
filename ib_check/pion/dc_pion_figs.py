"""
dc_pion_figs.py : pion DC fiducial cuts on edge, common note Sec. 5.4 (Fig. 5.14), IB and OB.

Usage (from the clas-framework folder):
    python3 ib_check/pion/dc_pion_figs.py /volatile/clas12/mtenorio/pi0/ib_check/pion

Reads   <dir>/root/pion_ana_<T>_<POL>.root, PionAnalysis/Pi{Plus,Minus}/chi2ndf_vs_edge_r{1,2,3}
          all sectors (sector-averaged, as Fig. 5.14), full-track chi2/NDF, edge 0-50 cm in 0.25 cm bins,
          chi2/NDF 0-100; filled after the pion pre-selection (FD, p >= 0.3, calorimeter energy, NDF > 0), no DC cut.
Method  same as the electron dc_figs.py (note Sec. 3.2.2): profile <chi2/ndf> vs edge, 5-point moving window over the
        bins with entries, scanning from large to small edge (start 20 cm for R1, R2 and 30 cm for R3); cut at the
        centre of the first window with variance > 0.05.
Writes  <dir>/dc/dc_pion_<POL>.png/.pdf   layout of Fig. 5.14: rows LD2, CxC, CuSn; columns R1, R2, R3;
          pi- blue, pi+ green, dashed = this work's cut, dotted = cut used in PionSelectionAlg (common note text).
Prints  cuts per polarity, target, charge and region, next to the Fig. 5.14 panel values (OB) and the code values.
Files that cannot be read (e.g. being rewritten by a running job) are skipped.
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import uproot

DIR = Path(sys.argv[1])
OUT = DIR / "dc"
OUT.mkdir(parents=True, exist_ok=True)
POLS, TARGETS, REGIONS = ["IB", "OB"], ["LD2", "CxC", "CuSn"], [1, 2, 3]
SPECIES = [("PiMinus", r"$\pi^-$", "#1f5fa8"), ("PiPlus", r"$\pi^+$", "#2ca02c")]
WIN, VAR_MAX, MIN_N = 5, 0.05, 1
EMAX = {1: 20.0, 2: 20.0, 3: 30.0}
CODE = {"PiMinus": {1: 2.125, 2: 3.625, 3: 9.625}, "PiPlus": {1: 3.875, 2: 4.375, 3: 2.625}}
FIG514 = {  # common note Fig. 5.14 panel values (OB), cm
    ("LD2", "PiMinus"): (2.12, 3.62, 9.62), ("LD2", "PiPlus"): (3.88, 4.38, 2.62),
    ("CxC", "PiMinus"): (2.12, 3.62, 10.12), ("CxC", "PiPlus"): (3.62, 3.88, 5.88),
    ("CuSn", "PiMinus"): (2.38, 3.62, 9.38), ("CuSn", "PiPlus"): (3.62, 4.38, 2.12),
}


def find(f, suffix):
    for k in f.keys(recursive=True, cycle=False):
        if k.endswith(suffix):
            return f[k]
    return None


def profile_2d(v, ex, ey):
    """Mean of y, its error (RMS/sqrt(N)) and N in each x bin (as dc_figs.py)."""
    xc, yc = 0.5 * (ex[1:] + ex[:-1]), 0.5 * (ey[1:] + ey[:-1])
    n = v.sum(axis=1)
    with np.errstate(invalid="ignore", divide="ignore"):
        m = (v * yc).sum(axis=1) / n
        rms = np.sqrt(np.clip((v * yc**2).sum(axis=1) / n - m**2, 0, None))
        e = rms / np.sqrt(n)
    return xc, m, e, n


def moving_variance_cut(x, m, n, r):
    """As dc_figs.py: first window above threshold, scanning from large edge."""
    ok = (n >= MIN_N) & (x < EMAX[r]) & np.isfinite(m)
    xs, ms = x[ok], m[ok]
    for i in range(len(xs) - WIN, -1, -1):
        if np.var(ms[i:i + WIN]) > VAR_MAX:
            return xs[i + WIN // 2]
    return np.nan


cuts = {}
for pol in POLS:
    fig, axs = plt.subplots(len(TARGETS), 3, figsize=(15, 3.6 * len(TARGETS)), squeeze=False)
    for i, t in enumerate(TARGETS):
        fn = DIR / "root" / f"pion_ana_{t}_{pol}.root"
        try:
            f = uproot.open(fn)
        except Exception as err:  # missing, or being rewritten
            print(f"skipped {fn}: {type(err).__name__}")
            f = None
        for j, r in enumerate(REGIONS):
            ax = axs[i, j]
            lines = []
            for sp, lab, col in SPECIES:
                h = find(f, f"PionAnalysis/{sp}/chi2ndf_vs_edge_r{r}") if f is not None else None
                if h is None:
                    continue
                x, m, e, n = profile_2d(h.values(), h.axis(0).edges(), h.axis(1).edges())
                c = moving_variance_cut(x, m, n, r)
                cuts[(pol, t, sp, r)] = c
                ok = n > 0
                ax.errorbar(x[ok], m[ok], yerr=e[ok], fmt="o", ms=1.8, lw=0.7, color=col, label=lab)
                if np.isfinite(c):
                    ax.axvline(c, color=col, ls="--", lw=1)
                ax.axvline(CODE[sp][r], color=col, ls=":", lw=1)
                lines.append(f"{lab}: {c:5.2f} cm" if np.isfinite(c) else f"{lab}:  none")
            if lines:
                ax.text(0.02, 0.97, "\n".join(lines), transform=ax.transAxes, va="top", ha="left", fontsize=8,
                        bbox=dict(boxstyle="round", fc="white", ec="gray", alpha=0.85))
                ax.legend(fontsize=8, loc="upper right", frameon=False)
            ax.set_xlim(0, 50)
            ax.set_title(f"{pol}, {t}, Region {r}", fontsize=10)
            ax.set_xlabel("edge [cm]")
            ax.set_ylabel(r"$\langle\chi^2/ndf\rangle$")
            ax.grid(alpha=0.3)
    fig.suptitle(f"{pol}: pion $\\chi^2$/ndf vs edge (all sectors); dashed = this work's cut, "
                 f"dotted = cut in PionSelectionAlg", fontsize=11)
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"dc_pion_{pol}.{ext}", dpi=140, bbox_inches="tight")
    plt.close(fig)
    print("wrote", OUT / f"dc_pion_{pol}.png")

print("\nPion DC edge cuts [cm]: this work | Fig. 5.14 (OB) | PionSelectionAlg (common note text)")
print(f"{'pol':3} {'target':6} {'species':8} | {'R1':>6} {'R2':>6} {'R3':>6} | {'R1':>6} {'R2':>6} {'R3':>6} | "
      f"{'R1':>6} {'R2':>6} {'R3':>6}")
for pol in POLS:
    for t in TARGETS:
        for sp, _, _ in SPECIES:
            ours = [cuts.get((pol, t, sp, r), np.nan) for r in REGIONS]
            if all(np.isnan(ours)):
                continue
            fig_v = FIG514[(t, sp)] if pol == "OB" else (np.nan,) * 3
            code = [CODE[sp][r] for r in REGIONS]
            print(f"{pol:3} {t:6} {sp:8} | " + " ".join(f"{v:6.2f}" for v in ours) + " | "
                  + " ".join(f"{v:6.2f}" if np.isfinite(v) else f"{'-':>6}" for v in fig_v) + " | "
                  + " ".join(f"{v:6.3f}" for v in code))
