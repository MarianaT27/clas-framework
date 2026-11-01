"""
fig3_4.py : IB version of the common-note Fig. 3.4 (SF vs E_tot) with the Event Builder mu +- 5 sigma band.

Usage:
    python3 fig3_4.py <root_dir> <out_dir>
    e.g. python3 fig3_4.py /volatile/clas12/mtenorio/pi0/ib_check/root /volatile/clas12/mtenorio/pi0/ib_check/sf

Reads:  <root_dir>/ib_fig_{LD2,CxC,CuSn}.root   Tracks/NegativeTracks/Etot_vs_SF_cut_S1 (N_phe > 2, E_PCal > 0.06 GeV)
Writes: <out_dir>/fig3_4_sf_etot.png/.pdf       sector 1, one panel per target, with mu(E) (red) and mu +- 5 sigma (black)
EB parameters: CCDB /calibration/eb/electron_sf, run 18335 ("based on 12857"), sector 1:
    mu(E)    = sf1  * (sf2  + sf3 /E + sf4 /E^2)
    sigma(E) = sfs1 * (sfs2 + sfs3/E + sfs4/E^2)      E = total deposited energy [GeV]
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm

if len(sys.argv) != 3:
    sys.exit("usage: python3 fig3_4.py <root_dir> <out_dir>")
ROOT, OUT = Path(sys.argv[1]), Path(sys.argv[2])
OUT.mkdir(parents=True, exist_ok=True)

TARGETS = {"LD2": "D (LD2), run 18335", "CxC": "C (CxC), run 18344", "CuSn": "CuSn, run 18384"}
# CCDB /calibration/eb/electron_sf, run 18335, sector 1: sf1..sf4, sfs1..sfs4
SF = (0.23372, 1.0, -0.01726, 0.00050)
SFS = (0.01763, 1.0, 0.0, 0.0)
N_SIGMA = 5.0

E = np.linspace(0.05, 2.0, 400)
mu = SF[0] * (SF[1] + SF[2] / E + SF[3] / E**2)
sg = SFS[0] * (SFS[1] + SFS[2] / E + SFS[3] / E**2)

fig, axs = plt.subplots(1, 3, figsize=(15, 4.4))
for ax, (t, lab) in zip(axs, TARGETS.items()):
    h = uproot.open(ROOT / f"ib_fig_{t}.root")["Tracks/NegativeTracks/Etot_vs_SF_cut_S1"]
    v, ex, ey = h.values(), h.axis(0).edges(), h.axis(1).edges()
    m = ax.pcolormesh(ex, ey, np.where(v > 0, v, np.nan).T, norm=LogNorm(), cmap="viridis")
    fig.colorbar(m, ax=ax)
    ax.plot(E, mu, color="red", lw=1.5, label=r"EB $\mu(E)$ (CCDB)")
    ax.plot(E, mu + N_SIGMA * sg, color="black", lw=1.2, label=r"$\mu \pm 5\sigma$")
    ax.plot(E, mu - N_SIGMA * sg, color="black", lw=1.2)
    ax.set_xlim(0, 2)
    ax.set_ylim(0, 0.5)
    ax.set_xlabel(r"$E_{dep,tot}$ [GeV]")
    ax.set_ylabel(r"SF = $E_{dep,tot}/p$")
    ax.set_title(f"{lab}, IB, sector 1: $N_{{phe}} > 2$, $E_{{PCal}} > 0.06$ GeV", fontsize=9)
    ax.legend(fontsize=7, loc="upper right")
fig.tight_layout()
for ext in ("png", "pdf"):
    fig.savefig(OUT / f"fig3_4_sf_etot.{ext}", dpi=140)
print(f"wrote {OUT}/fig3_4_sf_etot.png/.pdf")
