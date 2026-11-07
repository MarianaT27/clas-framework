"""
dvz_figs.py : pion Delta-Vz check, common note Sec. 5.3 (Eq. 13, Table 9, Fig. 5.13).

Usage (from the clas-framework folder, after pion_analysis with the v1.2 histograms):
    python3 ib_check/pion/dvz_figs.py /volatile/clas12/mtenorio/pi0/ib_check/pion

Reads   <dir>/root/pion_ana_<T>_<POL>.root, PionAnalysis/Pi{Plus,Minus}/delta_vz_e_pi[_up|_down]
          Delta vz = vz(e-) - vz(pi), raw vertices, 0.1 cm bins.
          Rows: LD2, CxC (all pions), Cu = CuSn upstream foil, Sn = CuSn downstream foil (pion raw vz < / >= -5.5 cm).
Method  as in the note: Gaussian fit over the top 50% of the peak (bins >= half the maximum, contiguous around the
        peak), selection window mu +- 3 sigma.
Writes  <dir>/dvz/dvz_<POL>.png/.pdf   layout of Fig. 5.13: rows LD2, CxC, Cu, Sn; columns pi-, pi+.
          Red = Gaussian fit (over its fit range), dashed = this work mu +- 3 sigma, dotted = Table 9 window (OB).
        Printed: mu, sigma and window per polarity, target and charge, next to Table 9 (OB only in the note).
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import uproot
from scipy.optimize import curve_fit

DIR = Path(sys.argv[1])
OUT = DIR / "dvz"
OUT.mkdir(parents=True, exist_ok=True)
POLS = ["IB", "OB"]
ROWS = [("LD2", "LD2", ""), ("CxC", "CxC", ""), ("Cu", "CuSn", "_up"), ("Sn", "CuSn", "_down")]
COLOR = {"LD2": "black", "CxC": "#1f77b4", "CuSn": "#9467bd"}
SPECIES = [("PiMinus", r"$\pi^-$"), ("PiPlus", r"$\pi^+$")]
TABLE9 = {  # common note Table 9, OB: (mu, sigma, window lo, window hi) [cm]
    ("LD2", "PiMinus"): (0.18, 1.34, -3.85, 4.21), ("LD2", "PiPlus"): (-0.85, 1.05, -3.99, 2.29),
    ("CxC", "PiMinus"): (0.19, 1.33, -3.81, 4.19), ("CxC", "PiPlus"): (-0.84, 1.05, -3.98, 2.30),
    ("Cu", "PiMinus"): (0.20, 1.34, -3.82, 4.21), ("Cu", "PiPlus"): (-0.85, 1.03, -3.95, 2.26),
    ("Sn", "PiMinus"): (0.17, 1.26, -3.60, 3.94), ("Sn", "PiPlus"): (-0.81, 1.04, -3.93, 2.30),
}


def find(f, suffix):
    for k in f.keys(recursive=True, cycle=False):
        if k.endswith(suffix):
            return f[k]
    return None


def gauss(x, a, m, s):
    return a * np.exp(-0.5 * ((x - m) / s) ** 2)


def fit_top_half(y, xc):
    ys = np.convolve(y, np.ones(3) / 3, mode="same")
    i0 = int(np.argmax(ys))
    half = 0.5 * ys[i0]
    lo = i0
    while lo > 0 and ys[lo - 1] >= half:
        lo -= 1
    hi = i0
    while hi < len(y) - 1 and ys[hi + 1] >= half:
        hi += 1
    x, yy = xc[lo:hi + 1], y[lo:hi + 1]
    s0 = max((xc[hi] - xc[lo]) / 2.355, 0.1)
    p, cov = curve_fit(gauss, x, yy, p0=[ys[i0], xc[i0], s0], sigma=np.sqrt(np.maximum(yy, 1)))
    p[2] = abs(p[2])
    chi2 = np.sum(((yy - gauss(x, *p)) / np.sqrt(np.maximum(yy, 1))) ** 2) / max(len(x) - 3, 1)
    return p, np.sqrt(np.diag(cov)), (xc[lo], xc[hi]), chi2


rows = []
for pol in POLS:
    fig, axs = plt.subplots(len(ROWS), 2, figsize=(10, 3.0 * len(ROWS)), squeeze=False)
    for i, (name, t, suf) in enumerate(ROWS):
        fn = DIR / "root" / f"pion_ana_{t}_{pol}.root"
        f = uproot.open(fn) if fn.exists() else None
        for j, (sp, lab) in enumerate(SPECIES):
            ax = axs[i, j]
            h = find(f, f"PionAnalysis/{sp}/delta_vz_e_pi{suf}") if f is not None else None
            if h is None:
                ax.text(0.5, 0.5, "no histogram", ha="center", transform=ax.transAxes)
                continue
            y, xe = h.values(), h.axis().edges()
            xc = 0.5 * (xe[1:] + xe[:-1])
            ax.stairs(y, xe, color=COLOR[t], lw=0.9)
            try:
                p, e, (flo, fhi), chi2 = fit_top_half(y, xc)
            except (RuntimeError, ValueError):
                ax.text(0.5, 0.5, "fit failed", ha="center", transform=ax.transAxes)
                continue
            m, s = p[1], p[2]
            xx = np.linspace(flo, fhi, 200)
            ax.plot(xx, gauss(xx, *p), color="red", lw=1.6,
                    label=rf"$\mu$={m:.3f}$\pm${e[1]:.3f}, $\sigma$={s:.3f}$\pm${e[2]:.3f} cm")
            for v in (m - 3 * s, m + 3 * s):
                ax.axvline(v, color="gray", ls="--", lw=0.9)
            note = TABLE9.get((name, sp)) if pol == "OB" else None
            if note:
                for v in note[2:]:
                    ax.axvline(v, color="green", ls=":", lw=1.1)
            ax.set_xlim(-20, 20)
            ax.set_title(rf"{pol}, {name}, {lab}", fontsize=10)
            ax.legend(fontsize=7, loc="upper right")
            ax.tick_params(labelsize=8)
            if i == len(ROWS) - 1:
                ax.set_xlabel(r"$\Delta V_z = V_z(e^-) - V_z(\pi)$ [cm]")
            rows.append((pol, name, sp, m, s, m - 3 * s, m + 3 * s, chi2, note))
    leg = "dashed = this work $\\mu\\pm3\\sigma$" + (", dotted = Table 9" if pol == "OB" else "")
    fig.suptitle(f"{pol}: $\\Delta V_z$, Gaussian fit over the top 50% of the peak (red); {leg}", fontsize=11)
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"dvz_{pol}.{ext}", dpi=140, bbox_inches="tight")
    plt.close(fig)
    print("wrote", OUT / f"dvz_{pol}.png")

print("\nDelta Vz = Vz(e-) - Vz(pi): this work vs common note Table 9 (OB only), cm")
print(f"{'pol':3} {'target':6} {'species':8} | {'mu':>6} {'sigma':>6} {'window':>16} {'chi2/ndf':>8} | "
      f"{'mu':>6} {'sigma':>6} {'window (Table 9)':>16}")
for pol, name, sp, m, s, lo, hi, chi2, note in rows:
    nt = f"{note[0]:6.2f} {note[1]:6.2f} [{note[2]:5.2f}, {note[3]:5.2f}]" if note else f"{'-':>6} {'-':>6} {'-':>16}"
    print(f"{pol:3} {name:6} {sp:8} | {m:6.2f} {s:6.2f} [{lo:6.2f}, {hi:5.2f}] {chi2:8.1f} | {nt}")
