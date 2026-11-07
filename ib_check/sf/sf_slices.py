"""
sf_slices.py : check of the SF vs p slice fits in sf_fits.py, for IB or OB, with the PCAL fiducial cut applied.

Usage (after sf_fits.py, same out_dir):
    python3 sf_slices.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 sf_slices.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/sf

Reads:  <root_dir>/<pol>_fig_{LD2,CxC,CuSn}.root   Tracks/ElectronTracks/p_vs_SF_pcalfid_S<s>
        <out_dir>/sf_fit_params_<pol>.csv            written by sf_fits.py
Writes: <out_dir>/check_sf_p_sectors_<T>.png/.pdf  SF vs p, all 6 sectors, with our fitted mu +- 3.5 sigma (solid)
                                                  and the code mu +- 3.5 sigma of this polarity (dashed)
        <out_dir>/check_sf_slices_LD2.png/.pdf    1D SF distributions for p in [2.86, 3.08), [5.94, 6.16), [7.92, 8.14),
                                                  [9.46, 9.68) GeV, sectors 1 to 6, with the Gaussian fit and its fit window,
                                                  and the note's mu (dashed) and mu +- sigma (dotted) of this polarity
Prints: entries per momentum slice (LD2, every 5th slice), per sector.
Same slicing and fit as sf_fits.py (common.sf_peak_fit): 50 slices from 0 to 11 GeV, Gaussian in code mean +- 0.02.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import (args, fname, label, N_SIGMA_SF, cubic, code_band, save, masked, gauss, sf_peak_fit)

ROOT, POL, OUT = args()
TARGETS = ["LD2", "CxC", "CuSn"]
HIST = "Tracks/ElectronTracks/p_vs_SF_pcalfid_S{}"
N_SLICES, P_MAX = 50, 11.0
EDGES = np.linspace(0, P_MAX, N_SLICES + 1)
SHOW_SLICES = [13, 27, 36, 43]          # p in [2.86,3.08), [5.94,6.16), [7.92,8.14), [9.46,9.68)
SHOW_SECTORS = [1, 2, 3, 4, 5, 6]


# our fitted parameters from sf_fits.py: target,run,sector,4 mu,4 sigma,n_slices,p_min,p_max
fitpar = {}
for line in (OUT / f"sf_fit_params_{POL}.csv").read_text().splitlines()[1:]:
    c = line.split(",")
    fitpar[(c[0], int(c[2]))] = ([float(x) for x in c[3:7]], [float(x) for x in c[7:11]], float(c[12]), float(c[13]))


def hist(f, s):
    h = f[HIST.format(s)]
    return h.values(), h.axis(0).edges(), h.axis(1).edges()


# ---- SF vs p, all sectors, per target
for t in TARGETS:
    f = uproot.open(fname(ROOT, POL, t))
    fig, axs = plt.subplots(2, 3, figsize=(16, 9))
    for s in range(1, 7):
        ax = axs.flat[s - 1]
        v, ep, es = hist(f, s)
        m = ax.pcolormesh(ep, es, masked(v), norm=LogNorm(), cmap="viridis")
        fig.colorbar(m, ax=ax)
        pmu, psg, plo, phi = fitpar[(t, s)]
        pp = np.linspace(0.5, 10.5, 300)
        mu, sg = cubic(pp, *pmu), cubic(pp, *psg)
        inr = (pp >= plo) & (pp <= phi)
        for k, (yy, lw) in enumerate(((mu, 1.5), (mu + N_SIGMA_SF * sg, 1), (mu - N_SIGMA_SF * sg, 1))):
            ax.plot(pp, np.where(inr, yy, np.nan), color="red", lw=lw,
                    label=rf"our fit $\mu \pm {N_SIGMA_SF}\sigma$ (dotted: extrapolation)" if k == 0 else None)
            ax.plot(pp, np.where(pp <= plo, yy, np.nan), color="red", lw=lw, ls=":")
            ax.plot(pp, np.where(pp >= phi, yy, np.nan), color="red", lw=lw, ls=":")
        pc = np.linspace(0.5, 11, 200)
        cm, cs = code_band(POL, s, pc)
        ax.plot(pc, cm, color="white", lw=1.2, ls="--", label=rf"code {POL} $\mu \pm {N_SIGMA_SF}\sigma$")
        ax.plot(pc, cm + N_SIGMA_SF * cs, color="white", lw=0.8, ls="--")
        ax.plot(pc, cm - N_SIGMA_SF * cs, color="white", lw=0.8, ls="--")
        ax.set_ylim(0.1, 0.35)
        ax.set_xlabel(r"$p$ [GeV]")
        ax.set_ylabel("SF")
        ax.set_title(f"{label(POL, t)}, sector {s}, PCAL fiducial", fontsize=9)
        if s == 1:
            ax.legend(fontsize=7, loc="lower left", facecolor="gray")
    save(fig, OUT, f"check_sf_p_sectors_{t}", dpi=130)

# ---- slice fits, LD2
f = uproot.open(fname(ROOT, POL, "LD2"))
fig, axs = plt.subplots(len(SHOW_SECTORS), len(SHOW_SLICES), figsize=(4 * len(SHOW_SLICES), 3.6 * len(SHOW_SECTORS)))
for r, s in enumerate(SHOW_SECTORS):
    v, ep, es = hist(f, s)
    sc, pc = 0.5 * (es[1:] + es[:-1]), 0.5 * (ep[1:] + ep[:-1])
    for c, k in enumerate(SHOW_SLICES):
        lo, hi = EDGES[k], EDGES[k + 1]
        y = v[(pc >= lo) & (pc < hi)].sum(axis=0)
        ax = axs[r, c]
        ax.step(sc, y, where="mid", color="black")
        txt = "fit failed"
        try:
            popt, _, w = sf_peak_fit(sc, y, code_band(POL, s, 0.5 * (lo + hi))[0])
            xx = np.linspace(sc[w][0], sc[w][-1], 200)
            ax.plot(xx, gauss(xx, *popt), color="red")
            ax.axvspan(sc[w][0], sc[w][-1], color="red", alpha=0.08)
            txt = rf"$\mu$ = {popt[1]:.4f}, $\sigma$ = {abs(popt[2]):.4f}"
        except (RuntimeError, ValueError, TypeError):
            pass
        cmu, csg = code_band(POL, s, 0.5 * (lo + hi))
        ax.axvline(cmu, color="gray", ls="--", label=f"note {POL} $\\mu$ = {cmu:.4f}")
        ax.axvline(cmu - csg, color="gray", ls=":", label=f"note {POL} $\\mu \\pm \\sigma$, $\\sigma$ = {csg:.4f}")
        ax.axvline(cmu + csg, color="gray", ls=":")
        ax.set_xlim(0.1, 0.35)
        ax.set_xlabel("SF")
        ax.set_title(f"S{s}, p $\\in$ [{lo:.2f}, {hi:.2f}) GeV, {int(y.sum())} entries\n{txt}", fontsize=8)
        ax.legend(fontsize=7, frameon=False)
fig.suptitle(f"{label(POL, 'LD2')}, PCAL fiducial", fontsize=10)
save(fig, OUT, "check_sf_slices_LD2", dpi=130)

# ---- statistics per slice
print(f"\n=== LD2 {POL}, PCAL fiducial: entries per momentum slice (every 5th slice), per sector")
print("p range [GeV]      " + "".join(f"     S{s}" for s in range(1, 7)))
vals = {s: hist(f, s) for s in range(1, 7)}
for k in range(0, N_SLICES, 5):
    lo, hi = EDGES[k], EDGES[k + 1]
    row = []
    for s in range(1, 7):
        v, ep, _ = vals[s]
        pc = 0.5 * (ep[1:] + ep[:-1])
        row.append(int(v[(pc >= lo) & (pc < hi)].sum()))
    print(f"[{lo:5.2f}, {hi:5.2f})    " + "".join(f"{n:8d}" for n in row))
