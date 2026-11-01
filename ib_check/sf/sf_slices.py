"""
sf_slices.py : check of the SF vs p slice fits in sf_fits.py (artifact or real?).

Usage:
    python3 sf_slices.py <root_dir> <out_dir>
    e.g. python3 sf_slices.py /volatile/clas12/mtenorio/pi0/ib_check/root /volatile/clas12/mtenorio/pi0/ib_check/sf

Reads:  <root_dir>/ib_fig_{LD2,CxC,CuSn}.root   ElectronTracks/p_vs_SF_S<s>
        <out_dir>/sf_fit_params_IB.csv            written by sf_fits.py
Writes: <out_dir>/check_sf_p_sectors_<T>.png/.pdf  SF vs p, all 6 sectors, with our fitted mu +- 3.5 sigma (solid)
                                                  and the code IB mu +- 3.5 sigma (dashed)
        <out_dir>/check_sf_slices_LD2.png/.pdf    1D SF distributions for p in [2.86, 3.08), [5.94, 6.16), [7.92, 8.14),
                                                  [9.46, 9.68) GeV, sectors 1 and 3, with the Gaussian fit and its +-2 sigma window
Prints: entries per momentum slice (LD2, every 5th slice), per sector, to show the statistics.
Same slicing and fit as sf_fits.py: 50 slices from 0 to 11 GeV, Gaussian in +-2 sigma around the peak, iterated twice.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from scipy.optimize import curve_fit

if len(sys.argv) != 3:
    sys.exit("usage: python3 sf_slices.py <root_dir> <out_dir>")
ROOT, OUT = Path(sys.argv[1]), Path(sys.argv[2])

TARGETS = {"LD2": "D (LD2), run 18335", "CxC": "C (CxC), run 18344", "CuSn": "CuSn, run 18384"}
N_SLICES, P_MAX = 50, 11.0
EDGES = np.linspace(0, P_MAX, N_SLICES + 1)
SHOW_SLICES = [13, 27, 36, 43]          # p in [2.86,3.08), [5.94,6.16), [7.92,8.14), [9.46,9.68)
SHOW_SECTORS = [1, 3]

CODE_MU = [[2.311088E-01, 4.309933E-03, -5.451627E-04, 4.965823E-06],
           [2.304337E-01, 3.585716E-03, -1.672223E-04, -7.317175E-06],
           [2.269123E-01, 8.797582E-03, -1.171416E-03, 3.051342E-05],
           [2.288870E-01, 6.786520E-03, -8.022602E-04, 1.557251E-05],
           [2.288645E-01, 4.637362E-03, -3.662892E-04, -4.233509E-06],
           [2.272093E-01, 6.581981E-03, -8.197177E-04, 1.632178E-05]]
CODE_SG = [[2.235239E-02, -3.117249E-03, 2.887205E-04, -1.130317E-05],
           [2.170118E-02, -2.428037E-03, 1.302147E-04, -2.503248E-09],
           [2.423467E-02, -4.301928E-03, 4.742880E-04, -1.983022E-05],
           [2.313626E-02, -3.260271E-03, 3.018634E-04, -1.156770E-05],
           [2.310895E-02, -3.892876E-03, 4.115173E-04, -1.656769E-05],
           [2.321748E-02, -3.665685E-03, 3.804588E-04, -1.587616E-05]]


def cubic(p, c):
    return c[0] + c[1] * p + c[2] * p**2 + c[3] * p**3


def gauss(x, n, mu, s):
    return n * np.exp(-0.5 * ((x - mu) / s) ** 2)


# our fitted parameters from sf_fits.py
fitpar = {}
for line in (OUT / "sf_fit_params_IB.csv").read_text().splitlines()[1:]:
    c = line.split(",")
    fitpar[(c[0], int(c[1]))] = ([float(x) for x in c[2:6]], [float(x) for x in c[6:10]], float(c[11]), float(c[12]))


def save(fig, name):
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=130)
    plt.close(fig)
    print(f"wrote {OUT / name}.png/.pdf")


def hist(f, s):
    h = f[f"Tracks/ElectronTracks/p_vs_SF_S{s}"]
    return h.values(), h.axis(0).edges(), h.axis(1).edges()


# ---- SF vs p, all sectors, per target
for t, lab in TARGETS.items():
    f = uproot.open(ROOT / f"ib_fig_{t}.root")
    fig, axs = plt.subplots(2, 3, figsize=(16, 9))
    for s in range(1, 7):
        ax = axs.flat[s - 1]
        v, ep, es = hist(f, s)
        m = ax.pcolormesh(ep, es, np.where(v > 0, v, np.nan).T, norm=LogNorm(), cmap="viridis")
        fig.colorbar(m, ax=ax)
        pmu, psg, plo, phi = fitpar[(t, s)]
        pp = np.linspace(plo, phi, 200)
        mu, sg = cubic(pp, pmu), cubic(pp, psg)
        ax.plot(pp, mu, color="red", lw=1.5, label=r"our fit $\mu \pm 3.5\sigma$")
        ax.plot(pp, mu + 3.5 * sg, color="red", lw=1)
        ax.plot(pp, mu - 3.5 * sg, color="red", lw=1)
        pc = np.linspace(0.5, 11, 200)
        cm, cs = cubic(pc, CODE_MU[s - 1]), cubic(pc, CODE_SG[s - 1])
        ax.plot(pc, cm, color="white", lw=1.2, ls="--", label=r"code IB $\mu \pm 3.5\sigma$")
        ax.plot(pc, cm + 3.5 * cs, color="white", lw=0.8, ls="--")
        ax.plot(pc, cm - 3.5 * cs, color="white", lw=0.8, ls="--")
        ax.set_ylim(0.1, 0.35)
        ax.set_xlabel(r"$p$ [GeV]")
        ax.set_ylabel(r"SF")
        ax.set_title(f"{lab}, IB, sector {s}", fontsize=9)
        if s == 1:
            ax.legend(fontsize=7, loc="lower left", facecolor="gray")
    save(fig, f"check_sf_p_sectors_{t}")

# ---- slice fits, LD2
f = uproot.open(ROOT / "ib_fig_LD2.root")
fig, axs = plt.subplots(len(SHOW_SECTORS), len(SHOW_SLICES), figsize=(4 * len(SHOW_SLICES), 3.6 * len(SHOW_SECTORS)))
for r, s in enumerate(SHOW_SECTORS):
    v, ep, es = hist(f, s)
    sc, pc = 0.5 * (es[1:] + es[:-1]), 0.5 * (ep[1:] + ep[:-1])
    for c, k in enumerate(SHOW_SLICES):
        lo, hi = EDGES[k], EDGES[k + 1]
        y = v[(pc >= lo) & (pc < hi)].sum(axis=0)
        ax = axs[r, c]
        ax.step(sc, y, where="mid", color="black")
        mu, sg = sc[np.argmax(y)], 0.02
        txt = "fit failed"
        try:
            for _ in range(2):
                w = (sc > mu - 2 * sg) & (sc < mu + 2 * sg) & (y > 0)
                popt, _ = curve_fit(gauss, sc[w], y[w], p0=[y[w].max(), mu, sg], sigma=np.sqrt(y[w]))
                mu, sg = popt[1], abs(popt[2])
            xx = np.linspace(sc[w][0], sc[w][-1], 200)
            ax.plot(xx, gauss(xx, *popt), color="red")
            ax.axvspan(sc[w][0], sc[w][-1], color="red", alpha=0.08)
            txt = rf"$\mu$ = {mu:.4f}, $\sigma$ = {sg:.4f}"
        except (RuntimeError, ValueError, TypeError):
            pass
        cmu = cubic(0.5 * (lo + hi), CODE_MU[s - 1])
        ax.axvline(cmu, color="gray", ls="--", label=f"code IB $\\mu$ = {cmu:.4f}")
        ax.set_xlim(0.1, 0.35)
        ax.set_xlabel("SF")
        ax.set_title(f"S{s}, p $\\in$ [{lo:.2f}, {hi:.2f}) GeV, {int(y.sum())} entries\n{txt}", fontsize=8)
        ax.legend(fontsize=7, frameon=False)
save(fig, "check_sf_slices_LD2")

# ---- statistics per slice
print("\n=== LD2 IB: entries per momentum slice (every 5th slice), per sector")
print("p range [GeV]      " + "".join(f"     S{s}" for s in range(1, 7)))
vals = {s: hist(f, s) for s in range(1, 7)}
for k in range(0, N_SLICES, 5):
    lo, hi = EDGES[k], EDGES[k + 1]
    row = []
    for s in range(1, 7):
        v, ep, _ = vals[s]
        pc = 0.5 * (ep[1:] + ep[:-1])
        row.append(int(v[(pc >= lo) & (pc < hi)].sum()))
    print(f"[{lo:5.2f}, {hi:5.2f})    " + "".join(f"{n:7d}" for n in row))
