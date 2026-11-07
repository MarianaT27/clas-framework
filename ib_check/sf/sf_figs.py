"""
sf_figs.py : common-note Figs. 3.1 to 3.5 and 3.9 (electron ID, sampling-fraction part), for IB or OB.

Usage:
    python3 sf_figs.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 sf_figs.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/sf

Reads:  <root_dir>/<ib|ob>_fig_{LD2,CxC,CuSn}.root   (electron_analysis, PCAL and DC cuts off)
        IB runs 18335, 18344, 18384; OB runs 18543, 18465, 18591
Writes: <out_dir>/fig3_1_nphe.png/.pdf        N_phe, all negative tracks in the FD with a PCAL hit, per sector, 0-75
        <out_dir>/fig3_2_epcal_ecal.png/.pdf  E_PCal vs E_ECal, sector 1: all negative tracks / N_phe > 2 / N_phe > 2 + SF cut
        <out_dir>/fig3_3_epcal.png/.pdf       E_PCal, sector 1: N_phe > 2 and N_phe > 2 + SF cut, 0.06 and 0.07 GeV lines,
                                              Gaussian fit to the pion MIP peak of the N_phe > 2 sample (prints mu + 7 sigma)
        <out_dir>/fig3_4_sf_etot.png/.pdf     SF vs E_tot, sector 1, N_phe > 2 and E_PCal > 0.06 GeV, with the Event Builder
                                              mu +- 5 sigma from CCDB (/calibration/eb/electron_sf, see CCDB_SF below)
        <out_dir>/fig3_5_sf_p.png/.pdf        SF vs p, sector 1, trigger electrons, with the code mu +- 3.5 sigma of this polarity
        <out_dir>/fig3_9_ecin_pcal.png/.pdf   E_ECin/p vs E_PCal/p, sector 1, trigger electrons, 5 < p < 6 GeV
Notes:
  - Histograms are filled by ElectronAnalysisAlg::process_tracks, which skips tracks with E_PCal = 0.
  - "SF cut" = N_phe > 2 and mu +- 3.5 sigma vs p with the parameters of the run's polarity (the code's cut).
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
from scipy.optimize import curve_fit

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import (args, fname, label, COLORS, SECTOR_COLORS, N_SIGMA_SF, code_band, save, h1, h2, masked)

ROOT, POL, OUT = args()
TARGETS = ["LD2", "CxC", "CuSn"]
files = {t: uproot.open(fname(ROOT, POL, t)) for t in TARGETS}

# CCDB /calibration/eb/electron_sf, sector 1: sf1..sf4, sfs1..sfs4; mu = sf1 (sf2 + sf3/E + sf4/E^2), same for sigma.
# IB: dump for run 18335 ("based on 12857"). OB: None until dumped for run 18543; the band is then not drawn.
CCDB_SF = {"IB": ((0.23372, 1.0, -0.01726, 0.00050), (0.01763, 1.0, 0.0, 0.0)),
           "OB": ((0.23372, 1.0, -0.01726, 0.00050), (0.01763, 1.0, 0.0, 0.0))}

# ---------------------------------------------------------------- Fig. 3.1
fig, axs = plt.subplots(1, 3, figsize=(15, 4.2))
for ax, t in zip(axs, TARGETS):
    for s in range(1, 7):
        v, e = h1(files[t], f"Tracks/NegativeTracks/nphe_S{s}")
        ax.step(e[:-1], v / max(v.sum(), 1), where="post", color=SECTOR_COLORS[s - 1], label=f"Sector {s}")
    ax.axvline(2.0, color="k", ls="--", label="Cut at 2.0")
    ax.set_xlim(0, 75)
    ax.set_xlabel(r"$N_{phe}$ (HTCC)")
    ax.set_ylabel("normalized counts")
    ax.set_title(f"{label(POL, t)}: all negative tracks in FD", fontsize=10)
    ax.legend(fontsize=7, frameon=False)
save(fig, OUT, "fig3_1_nphe")

# ---------------------------------------------------------------- Fig. 3.2
panels = (("EPCal_vs_ECal_S1", "all negative tracks"),
          ("EPCal_vs_ECal_cut_S1", r"$N_{phe} > 2$"),
          ("EPCal_vs_ECal_sfcut_S1", r"$N_{phe} > 2$ + SF cut"))
fig, axs = plt.subplots(3, 3, figsize=(15, 12))
for row, t in enumerate(TARGETS):
    for col, (name, what) in enumerate(panels):
        v, ex, ey = h2(files[t], f"Tracks/NegativeTracks/{name}")
        ax = axs[row, col]
        m = ax.pcolormesh(ex, ey, masked(v), norm=LogNorm(), cmap="viridis")
        fig.colorbar(m, ax=ax)
        ax.axvline(0.06, color="k", lw=1.5, label=r"$E_{PCal}$ = 60 MeV")
        ax.set_xlabel(r"$E_{PCal}$ [GeV]")
        ax.set_ylabel(r"$E_{ECin} + E_{ECout}$ [GeV]")
        ax.set_title(f"{label(POL, t)}, sector 1: {what}", fontsize=9)
        ax.legend(fontsize=7, loc="upper right")
save(fig, OUT, "fig3_2_epcal_ecal")


# ---------------------------------------------------------------- Fig. 3.3
def gauss(x, a, mu, s):
    return a * np.exp(-0.5 * ((x - mu) / s) ** 2)


fig, axs = plt.subplots(1, 3, figsize=(15, 4.2))
print(f"\n=== Fig. 3.3 ({POL}): Gaussian fit to the pion MIP peak in E_PCal (sector 1, N_phe > 2)")
for ax, t in zip(axs, TARGETS):
    v, e = h1(files[t], "Tracks/NegativeTracks/EPCal_cut_S1")
    vs, _ = h1(files[t], "Tracks/NegativeTracks/EPCal_sfcut_S1")
    c = 0.5 * (e[1:] + e[:-1])
    ax.step(e[:-1], v, where="post", color=COLORS[t], label=r"$N_{phe} > 2$")
    ax.step(e[:-1], vs, where="post", color=COLORS[t], ls=":", label=r"$N_{phe} > 2$ + SF cut")
    # MIP peak: the maximum below 0.05 GeV, fitted within +-0.012 GeV of it
    low = c < 0.05
    ipk = np.argmax(np.where(low, v, -1))
    win = (c > c[ipk] - 0.012) & (c < c[ipk] + 0.012) & (v > 0)
    txt = "MIP fit failed"
    try:
        p, _ = curve_fit(gauss, c[win], v[win], p0=[v[ipk], c[ipk], 0.005], sigma=np.sqrt(v[win]))
        xx = np.linspace(c[win][0], c[win][-1], 200)
        ax.plot(xx, gauss(xx, *p), color="orange")
        n7 = p[1] + 7 * abs(p[2])
        txt = rf"MIP: $\mu$ = {1000*p[1]:.1f} MeV, $\sigma$ = {1000*abs(p[2]):.1f} MeV, $\mu+7\sigma$ = {1000*n7:.1f} MeV"
        print(f"    {t:5s} mu = {p[1]:.4f} GeV, sigma = {abs(p[2]):.4f} GeV, mu + 7 sigma = {n7:.4f} GeV, "
              f"0.06 GeV is {(0.06 - p[1]) / abs(p[2]):.1f} sigma above the peak")
    except (RuntimeError, ValueError):
        print(f"    {t:5s} MIP fit failed")
    ax.axvline(0.06, color="tab:orange", ls="--", label="RG-D cut 60 MeV")
    ax.axvline(0.07, color="purple", ls="--", label="RG-A cut 70 MeV")
    ax.set_xlim(0, 0.2)
    ax.set_xlabel(r"$E_{PCal}$ [GeV]")
    ax.set_ylabel("counts")
    ax.set_title(f"{label(POL, t)}, sector 1\n{txt}", fontsize=9)
    ax.legend(fontsize=7, frameon=False)
save(fig, OUT, "fig3_3_epcal")

# ---------------------------------------------------------------- Fig. 3.4
E = np.linspace(0.05, 2.0, 400)
fig, axs = plt.subplots(1, 3, figsize=(15, 4.4))
for ax, t in zip(axs, TARGETS):
    v, ex, ey = h2(files[t], "Tracks/NegativeTracks/Etot_vs_SF_cut_S1")
    m = ax.pcolormesh(ex, ey, masked(v), norm=LogNorm(), cmap="viridis")
    fig.colorbar(m, ax=ax)
    if CCDB_SF[POL] is not None:
        sf, sfs = CCDB_SF[POL]
        mu = sf[0] * (sf[1] + sf[2] / E + sf[3] / E**2)
        sg = sfs[0] * (sfs[1] + sfs[2] / E + sfs[3] / E**2)
        ax.plot(E, mu, color="red", lw=1.5, label=r"EB $\mu(E)$ (CCDB)")
        ax.plot(E, mu + 5 * sg, color="black", lw=1.2, label=r"$\mu \pm 5\sigma$")
        ax.plot(E, mu - 5 * sg, color="black", lw=1.2)
        ax.legend(fontsize=7, loc="upper right")
    ax.set_xlim(0, 2)
    ax.set_ylim(0, 0.5)
    ax.set_xlabel(r"$E_{dep,tot}$ [GeV]")
    ax.set_ylabel(r"SF = $E_{dep,tot}/p$")
    ax.set_title(f"{label(POL, t)}, sector 1: $N_{{phe}} > 2$, $E_{{PCal}} > 0.06$ GeV", fontsize=9)
if CCDB_SF[POL] is None:
    print(f"\nFig. 3.4 ({POL}): no CCDB parameters set in CCDB_SF, the EB band is not drawn.")
save(fig, OUT, "fig3_4_sf_etot")

# ---------------------------------------------------------------- Fig. 3.5
fig, axs = plt.subplots(1, 3, figsize=(15, 4.4))
pp = np.linspace(0.5, 11, 300)
mu, sg = code_band(POL, 1, pp)
for ax, t in zip(axs, TARGETS):
    v, ex, ey = h2(files[t], "Tracks/ElectronTracks/p_vs_SF_S1")
    m = ax.pcolormesh(ex, ey, masked(v), norm=LogNorm(), cmap="viridis")
    fig.colorbar(m, ax=ax)
    ax.plot(pp, mu, color="red", lw=1.5, label=rf"code {POL} $\mu_{{SF}}(p)$, S1")
    ax.plot(pp, mu + N_SIGMA_SF * sg, color="red", lw=1, ls="--", label=rf"$\mu \pm {N_SIGMA_SF}\sigma$")
    ax.plot(pp, mu - N_SIGMA_SF * sg, color="red", lw=1, ls="--")
    ax.set_xlabel(r"$p$ [GeV]")
    ax.set_ylabel(r"SF = $E_{dep,tot}/p$")
    ax.set_title(f"{label(POL, t)}, sector 1: trigger electrons", fontsize=9)
    ax.legend(fontsize=7, loc="lower right")
save(fig, OUT, "fig3_5_sf_p")

# ---------------------------------------------------------------- Fig. 3.9
fig, axs = plt.subplots(1, 3, figsize=(15, 4.4))
for ax, t in zip(axs, TARGETS):
    v, ex, ey = h2(files[t], "Tracks/ElectronTracks/ECin_SF_vs_PCal_SF_S1")
    m = ax.pcolormesh(ex, ey, masked(v), norm=LogNorm(), cmap="viridis")
    fig.colorbar(m, ax=ax)
    ax.set_xlabel(r"$E_{PCal}/p$")
    ax.set_ylabel(r"$E_{ECin}/p$")
    ax.set_title(f"{label(POL, t)}, sector 1: trigger electrons, $5 < p < 6$ GeV", fontsize=9)
save(fig, OUT, "fig3_9_ecin_pcal")
