"""
sf_calo_appendix.py : appendix figures of the common note in the note's style, for IB or OB, PDF only.
  Appendix D: target-dependent sampling fraction profiles, mu(p) and sigma(p) per sector + average, 1-sigma bands.
  Appendix E: calorimeter fiducial cut, profile of SF vs PCAL l_v, all sectors, 9 cm cut.

Usage:
    python3 ib_check/sf/sf_calo_appendix.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 ib_check/sf/sf_calo_appendix.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/appendix

Appendix D: same fits as sf_fits.py (PCAL fiducial cut, 50 momentum slices 0-11 GeV, fixed-window Gaussian per slice
  via common.sf_peak_fit, cubic fits of mu(p) and sigma(p) using the slices with 1.8 <= p <= 9.0 GeV), so the
  parameters are the ones of the IB SF table. The cubic is drawn from 0.7 to 10.9 GeV as in the note's OB figure
  (outside 1.8-9.0 GeV it is an extrapolation). Band = 1 sigma from the covariance of the cubic fit (curve_fit
  default, as in sf_fits.py). Average = mean of the six sector curves (as sf_fits.py).
  Reads Tracks/ElectronTracks/p_vs_SF_pcalfid_S<s>.
Appendix E: mean SF and error (RMS / sqrt(N)) in each l_v bin with >= 20 entries, trigger electrons before cuts
  (Tracks/ElectronTracks/lv_vs_SF_S<s>, as calo_figs.py).
Writes: <out_dir>/sf_profile_<T>.pdf, <out_dir>/sf_vs_lv_<T>.pdf  (T = LD2, CxC, CuSn)
Prints: the fitted parameters per target and sector (to compare with the SF table).
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import font_manager
from matplotlib.lines import Line2D
import numpy as np
import uproot
from scipy.optimize import curve_fit

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import args, fname, cubic, code_band, sf_peak_fit

ROOT, POL, OUT = args()
TARGETS = ["LD2", "CxC", "CuSn"]
TITLE = {"LD2": r"LD$_2$", "CxC": r"C $\times$ C", "CuSn": r"Cu $\times$ Sn"}
# sector colours of the note's appendix figures
SEC_COL = ["#1f5aa6", "#10b04f", "#ff9c1a", "#ff3311", "#8a5ca8", "#444444"]
HIST = "Tracks/ElectronTracks/p_vs_SF_pcalfid_S{}"
N_SLICES, P_MAX, MIN_ENTRIES = 50, 11.0, 200
P_FIT = (1.8, 9.0)
P_DRAW = np.linspace(0.7, 10.9, 300)
MIN_N, CUT = 20, 9.0

_avail = {f.name for f in font_manager.fontManager.ttflist}
_serif = [f for f in ("Latin Modern Roman", "LM Roman 10", "CMU Serif", "cmr10") if f in _avail] or ["DejaVu Serif"]
plt.rcParams.update({
    "font.family": "serif", "font.serif": _serif, "mathtext.fontset": "cm", "axes.formatter.use_mathtext": True,
    "font.size": 15, "axes.titlesize": 17, "axes.labelsize": 17, "legend.fontsize": 12,
    "xtick.direction": "in", "ytick.direction": "in", "xtick.top": False, "ytick.right": False,
    "xtick.minor.visible": True, "ytick.minor.visible": True,
    "xtick.major.size": 6, "ytick.major.size": 6, "xtick.minor.size": 3, "ytick.minor.size": 3,
    "axes.linewidth": 1.5, "axes.unicode_minus": False,
})
OUT.mkdir(parents=True, exist_ok=True)


def save_pdf(fig, name):
    fig.tight_layout()
    fig.savefig(OUT / f"{name}.pdf")
    plt.close(fig)
    print("wrote", OUT / f"{name}.pdf")


def slice_fits(f, sector):
    h = f[HIST.format(sector)]
    v, ep, es = h.values(), h.axis(0).edges(), h.axis(1).edges()
    sc, pc = 0.5 * (es[1:] + es[:-1]), 0.5 * (ep[1:] + ep[:-1])
    edges = np.linspace(0, P_MAX, N_SLICES + 1)
    out = []
    for lo, hi in zip(edges[:-1], edges[1:]):
        y = v[(pc >= lo) & (pc < hi)].sum(axis=0)
        if y.sum() < MIN_ENTRIES:
            continue
        try:
            popt, pcov, _ = sf_peak_fit(sc, y, code_band(POL, sector, 0.5 * (lo + hi))[0])
            err = np.sqrt(np.diag(pcov))
            out.append((0.5 * (lo + hi), popt[1], err[1], abs(popt[2]), err[2]))
        except (RuntimeError, ValueError):
            continue
    return np.array(out)


def curve_band(p, par, cov):
    J = np.vstack([np.ones_like(p), p, p**2, p**3]).T
    return cubic(p, *par), np.sqrt(np.einsum("ij,jk,ik->i", J, cov, J))


# ---- Appendix D
print("target,sector,a_mu,b_mu,c_mu,d_mu,a_sig,b_sig,c_sig,d_sig")
for t in TARGETS:
    f = uproot.open(fname(ROOT, POL, t))
    fig, (ax_m, ax_s) = plt.subplots(2, 1, figsize=(10, 7.6), sharex=True)
    mus, sgs = [], []
    for s in range(1, 7):
        pts = slice_fits(f, s)
        pts = pts[(pts[:, 0] >= P_FIT[0]) & (pts[:, 0] <= P_FIT[1])]
        pmu, cmu = curve_fit(cubic, pts[:, 0], pts[:, 1], sigma=pts[:, 2])
        psg, csg = curve_fit(cubic, pts[:, 0], pts[:, 3], sigma=pts[:, 4])
        print(f"{t},{s}," + ",".join(f"{x:.6E}" for x in (*pmu, *psg)))
        for ax, par, cov, store in ((ax_m, pmu, cmu, mus), (ax_s, psg, csg, sgs)):
            y, e = curve_band(P_DRAW, par, cov)
            store.append(y)
            ax.fill_between(P_DRAW, y - e, y + e, color=SEC_COL[s - 1], alpha=0.15, lw=0)
            ax.plot(P_DRAW, y, color=SEC_COL[s - 1], lw=2, label=f"Sector {s}")
    for ax, store in ((ax_m, mus), (ax_s, sgs)):
        ax.plot(P_DRAW, np.mean(store, axis=0), color="black", ls="--", lw=2.5, label="Average")
        leg = ax.legend(loc="upper right", ncol=2, frameon=True, fancybox=False, edgecolor="black")
        leg.get_frame().set_linewidth(1.2)
        ax.set_xlim(P_DRAW[0], P_DRAW[-1])
    ax_m.set_ylabel(r"$\mu\ (E_{\mathrm{dep,tot}}/p)$")
    ax_s.set_ylabel(r"$\sigma\ (E_{\mathrm{dep,tot}}/p)$")
    ax_s.set_xlabel(r"$p$ [GeV/$c$]", loc="right")
    ax_m.set_title(f"{TITLE[t]} Target")
    save_pdf(fig, f"sf_profile_{t}")


# ---- Appendix E
def lv_profile(f, s):
    h = f[f"Tracks/ElectronTracks/lv_vs_SF_S{s}"]
    v, ex, ey = h.values(), h.axis(0).edges(), h.axis(1).edges()
    xc, yc = 0.5 * (ex[1:] + ex[:-1]), 0.5 * (ey[1:] + ey[:-1])
    n = v.sum(axis=1)
    with np.errstate(invalid="ignore", divide="ignore"):
        m = (v * yc).sum(axis=1) / n
        e = np.sqrt(np.clip((v * yc**2).sum(axis=1) / n - m**2, 0, None)) / np.sqrt(n)
    ok = n >= MIN_N
    return xc[ok], m[ok], e[ok]


for t in TARGETS:
    f = uproot.open(fname(ROOT, POL, t))
    fig, ax = plt.subplots(figsize=(10, 7.7))
    ax.axvline(CUT, color="#444444", ls="--", lw=2.5, label="9 cm cut")
    for s in range(1, 7):
        x, m, e = lv_profile(f, s)
        ax.errorbar(x, m, yerr=e, fmt="o-", ms=4, lw=1.3, elinewidth=1.3, color=SEC_COL[s - 1], label=f"Sector {s}")
    ax.grid(True, ls=":", alpha=0.4)
    ax.set_xlim(0, 73)
    ax.set_ylabel("Sampling Fraction")
    ax.set_xlabel(r"$l_v$ [cm]", loc="right")
    ax.set_title(f"{TITLE[t]} - All Sectors")
    ax.legend(loc="lower right", ncol=2, frameon=False)
    save_pdf(fig, f"sf_vs_lv_{t}")
