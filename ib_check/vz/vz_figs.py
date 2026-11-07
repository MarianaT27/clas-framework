"""
vz_figs.py : common-note Fig. 3.11 (per-sector Vz migration from the correction), for IB or OB.

Usage:
    python3 vz_figs.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 vz_figs.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/vz

Reads:  <root_dir>/<ib|ob>_fig_{CxC,CuSn}.root   Tracks/ElectronTracks/vz_raw_vs_vz_corr_S<s>
        (trigger electrons, before cuts; raw vz on x, corrected vz on y, 260 x 260 bins in [-12, 1] cm)
        IB runs 18344 (CxC), 18384 (CuSn); OB runs 18465 (CxC), 18591 (CuSn). LD2 has no correction and is skipped.
Writes: <out_dir>/fig3_11_vz_migration_<T>.png/.pdf
        Same layout as macros/fig311.C: top row = raw vz per sector, split into 10 contiguous 1 cm intervals in
        [-10.5, -0.5] cm (stacked, one colour each, dotted interval boundaries); bottom row = corrected vz of the same
        electrons, keeping each interval's colour. The code's Voigt windows (mu +- 3 sigma_eff, VzCorrectorService)
        are drawn as dashed lines on the bottom row.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import args, fname, label, save

ROOT, POL, OUT = args()
TARGETS = ["CxC", "CuSn"]
VZ_LO, VZ_HI, N_INT = -10.5, -0.5, 10
XLIM = (-11.0, 0.5)
INT_COLORS = ["#1f4e9c", "#f28e2b", "#2ca02c", "#d62728", "#9467bd",
              "#8c564b", "#e377c2", "#7f7f7f", "#bcbd22", "#17becf"]

# VzCorrectorService::voigt_window (mu_lo, sigma_lo, mu_hi, sigma_hi); cut = mu +- 3 sigma_eff on corrected vz
WINDOWS = {("CuSn", "OB"): (-2.916, 0.370, -7.861, 0.415), ("CuSn", "IB"): (-2.812, 0.315, -7.765, 0.321),
           ("CxC", "OB"): (-2.906, 0.373, -7.887, 0.395), ("CxC", "IB"): (-2.713, 0.307, -7.670, 0.313)}
N_SIGMA = 3.0

edges_int = np.linspace(VZ_LO, VZ_HI, N_INT + 1)

for t in TARGETS:
    f = uproot.open(fname(ROOT, POL, t))
    fig, axs = plt.subplots(2, 6, figsize=(24, 7.5), sharex=True)
    mu_lo, s_lo, mu_hi, s_hi = WINDOWS[(t, POL)]
    for s in range(1, 7):
        h = f[f"Tracks/ElectronTracks/vz_raw_vs_vz_corr_S{s}"]
        v, ex, ey = h.values(), h.axis(0).edges(), h.axis(1).edges()
        xc = 0.5 * (ex[1:] + ex[:-1])
        top, bot = axs[0, s - 1], axs[1, s - 1]
        base_raw = np.zeros(len(ex) - 1)
        base_cor = np.zeros(len(ey) - 1)
        for k in range(N_INT):
            sel = (xc >= edges_int[k]) & (xc < edges_int[k + 1])
            raw = np.where(sel, v.sum(axis=1), 0.0)       # raw vz of this interval
            cor = v[sel].sum(axis=0)                       # corrected vz of the same electrons
            top.stairs(base_raw + raw, ex, baseline=base_raw, fill=True, color=INT_COLORS[k])
            bot.stairs(base_cor + cor, ey, baseline=base_cor, fill=True, color=INT_COLORS[k])
            base_raw += raw
            base_cor += cor
        for x in edges_int:
            top.axvline(x, color="k", ls=":", lw=0.6)
        for m, sg, nm in ((mu_lo, s_lo, "lo"), (mu_hi, s_hi, "hi")):
            for x in (m - N_SIGMA * sg, m + N_SIGMA * sg):
                bot.axvline(x, color="k", ls="--", lw=0.8)
        top.set_title(f"S{s} before (raw $v_z$)", fontsize=9)
        bot.set_title(f"S{s} after (corrected $v_z$)", fontsize=9)
        bot.set_xlabel(r"$v_z$ [cm]")
        for ax in (top, bot):
            ax.set_xlim(*XLIM)
            ax.set_ylim(bottom=0)
    axs[0, 0].set_ylabel("counts")
    axs[1, 0].set_ylabel("counts")
    axs[1, 5].plot([], [], color="k", ls="--", lw=0.8, label=rf"code window $\mu \pm {N_SIGMA:g}\sigma_{{eff}}$")
    axs[1, 5].legend(fontsize=7, loc="upper left", frameon=False)
    fig.suptitle(f"{label(POL, t)}: trigger electrons, before cuts", fontsize=11)
    save(fig, OUT, f"fig3_11_vz_migration_{t}")
