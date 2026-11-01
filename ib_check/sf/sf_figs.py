"""
sf_figs.py : IB versions of the common-note Figs. 3.1 to 3.5 (electron ID, sampling-fraction part).

Usage (from anywhere):
    python3 sf_figs.py /volatile/clas12/mtenorio/pi0/ib_check/root /volatile/clas12/mtenorio/pi0/ib_check/sf

Reads:  <root_dir>/ib_fig_{LD2,CxC,CuSn}.root   (electron_analysis, IB runs 18335, 18344, 18384)
Writes: <out_dir>/fig3_1_nphe.png/.pdf          N_phe, all negative tracks in the FD with a PCAL hit, per sector, per target
        <out_dir>/fig3_2_epcal_ecal.png/.pdf    E_PCal vs E_ECal, sector 1: all negative tracks / after N_phe > 2
        <out_dir>/fig3_3_epcal.png/.pdf         E_PCal, sector 1, after N_phe > 2, with the 0.06 and 0.07 GeV lines
                                                and a Gaussian fit to the pion MIP peak (prints mu + 7 sigma)
        <out_dir>/fig3_4_sf_etot.png/.pdf       SF vs E_tot, sector 1, after N_phe > 2 and E_PCal > 0.06 GeV
        <out_dir>/fig3_5_sf_p.png/.pdf          SF vs p, sector 1, trigger electrons, with the IB mu +- 3.5 sigma
                                                curves the code uses (SamplingFractionService, "IB", sector 1)
Notes:
  - Histograms are filled by ElectronAnalysisAlg::process_tracks, which skips tracks with E_PCal = 0.
  - The "_cut" histograms in the code require N_phe > 2 only (Figs. 3.2b and 3.3 in the note say N_phe + SF).
  - Fig. 3.4: the +-5 sigma curves use CCDB parameters, which are not in the ROOT file, so they are not drawn.
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
    sys.exit("usage: python3 sf_figs.py <root_dir> <out_dir>")
ROOT, OUT = Path(sys.argv[1]), Path(sys.argv[2])
OUT.mkdir(parents=True, exist_ok=True)

TARGETS = {"LD2": ("D (LD2), run 18335", "black"),
           "CxC": ("C (CxC), run 18344", "#1f77b4"),
           "CuSn": ("CuSn, run 18384", "#9467bd")}
SECTOR_COLORS = plt.get_cmap("tab10").colors[:6]

# SamplingFractionService.hpp, m_data["IB"], sector 1 (= note Table 14 "Average" S1)
SF_IB_S1_MU = [2.311088E-01, 4.309933E-03, -5.451627E-04, 4.965823E-06]
SF_IB_S1_SIG = [2.235239E-02, -3.117249E-03, 2.887205E-04, -1.130317E-05]
N_SIGMA_SF = 3.5

files = {t: uproot.open(ROOT / f"ib_fig_{t}.root") for t in TARGETS}


def h1(f, name):
    h = f[name]
    return h.values(), h.axis().edges()


def h2(f, name):
    h = f[name]
    return h.values(), h.axis(0).edges(), h.axis(1).edges()


def save(fig, name):
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=140)
    plt.close(fig)
    print(f"wrote {OUT / name}.png/.pdf")


def poly(c, p):
    return c[0] + c[1] * p + c[2] * p**2 + c[3] * p**3


# ---------------------------------------------------------------- Fig. 3.1
fig, axs = plt.subplots(1, 3, figsize=(15, 4.2))
for ax, (t, (lab, _)) in zip(axs, TARGETS.items()):
    for s in range(1, 7):
        v, e = h1(files[t], f"Tracks/NegativeTracks/nphe_S{s}")
        ax.step(e[:-1], v / max(v.sum(), 1), where="post", color=SECTOR_COLORS[s - 1], label=f"Sector {s}")
    ax.axvline(2.0, color="k", ls="--", label="Cut at 2.0")
    ax.set_yscale("log")
    ax.set_xlim(0, 50)
    ax.set_xlabel(r"$N_{phe}$ (HTCC)")
    ax.set_ylabel("normalized counts")
    ax.set_title(f"{lab}: all negative tracks in FD, IB", fontsize=10)
    ax.legend(fontsize=7, frameon=False)
save(fig, "fig3_1_nphe")

# ---------------------------------------------------------------- Fig. 3.2
fig, axs = plt.subplots(3, 2, figsize=(10, 12))
for row, (t, (lab, _)) in enumerate(TARGETS.items()):
    for col, (name, what) in enumerate((("EPCal_vs_ECal_S1", "all negative tracks"),
                                         ("EPCal_vs_ECal_cut_S1", r"$N_{phe} > 2$"))):
        v, ex, ey = h2(files[t], f"Tracks/NegativeTracks/{name}")
        ax = axs[row, col]
        m = ax.pcolormesh(ex, ey, np.where(v > 0, v, np.nan).T, norm=LogNorm(), cmap="viridis")
        fig.colorbar(m, ax=ax)
        ax.axvline(0.06, color="k", lw=1.5, label=r"$E_{PCal}$ = 60 MeV")
        ax.set_xlabel(r"$E_{PCal}$ [GeV]")
        ax.set_ylabel(r"$E_{ECin} + E_{ECout}$ [GeV]")
        ax.set_title(f"{lab}, IB, sector 1: {what}", fontsize=9)
        ax.legend(fontsize=7, loc="upper right")
save(fig, "fig3_2_epcal_ecal")


# ---------------------------------------------------------------- Fig. 3.3
def gauss(x, a, mu, s):
    return a * np.exp(-0.5 * ((x - mu) / s) ** 2)


fig, axs = plt.subplots(1, 3, figsize=(15, 4.2))
print("\n=== Fig. 3.3: Gaussian fit to the pion MIP peak in E_PCal (sector 1, N_phe > 2)")
for ax, (t, (lab, col)) in zip(axs, TARGETS.items()):
    v, e = h1(files[t], "Tracks/NegativeTracks/EPCal_cut_S1")
    c = 0.5 * (e[1:] + e[:-1])
    ax.step(e[:-1], v, where="post", color=col)
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
    ax.set_title(f"{lab}, IB, sector 1, $N_{{phe}} > 2$\n{txt}", fontsize=9)
    ax.legend(fontsize=7, frameon=False)
save(fig, "fig3_3_epcal")

# ---------------------------------------------------------------- Fig. 3.4
fig, axs = plt.subplots(1, 3, figsize=(15, 4.4))
for ax, (t, (lab, _)) in zip(axs, TARGETS.items()):
    v, ex, ey = h2(files[t], "Tracks/NegativeTracks/Etot_vs_SF_cut_S1")
    m = ax.pcolormesh(ex, ey, np.where(v > 0, v, np.nan).T, norm=LogNorm(), cmap="viridis")
    fig.colorbar(m, ax=ax)
    ax.set_xlabel(r"$E_{dep,tot}$ [GeV]")
    ax.set_ylabel(r"SF = $E_{dep,tot}/p$")
    ax.set_title(f"{lab}, IB, sector 1: $N_{{phe}} > 2$, $E_{{PCal}} > 0.06$ GeV", fontsize=9)
save(fig, "fig3_4_sf_etot")

# ---------------------------------------------------------------- Fig. 3.5
fig, axs = plt.subplots(1, 3, figsize=(15, 4.4))
pp = np.linspace(0.5, 11, 300)
mu, sg = poly(SF_IB_S1_MU, pp), poly(SF_IB_S1_SIG, pp)
for ax, (t, (lab, _)) in zip(axs, TARGETS.items()):
    v, ex, ey = h2(files[t], "Tracks/ElectronTracks/p_vs_SF_S1")
    m = ax.pcolormesh(ex, ey, np.where(v > 0, v, np.nan).T, norm=LogNorm(), cmap="viridis")
    fig.colorbar(m, ax=ax)
    ax.plot(pp, mu, color="red", lw=1.5, label=r"code IB $\mu_{SF}(p)$, S1")
    ax.plot(pp, mu + N_SIGMA_SF * sg, color="red", lw=1, ls="--", label=r"$\mu \pm 3.5\sigma$")
    ax.plot(pp, mu - N_SIGMA_SF * sg, color="red", lw=1, ls="--")
    ax.set_xlabel(r"$p$ [GeV]")
    ax.set_ylabel(r"SF = $E_{dep,tot}/p$")
    ax.set_title(f"{lab}, IB, sector 1: trigger electrons", fontsize=9)
    ax.legend(fontsize=7, loc="lower right")
save(fig, "fig3_5_sf_p")
