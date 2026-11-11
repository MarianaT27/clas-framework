"""
vz_figs.py : pion Vz check, common note Sec. 5.2 (Fig. 5.12, Table 8).

Usage (from the clas-framework folder, after pion_analysis with the v1.1 histograms):
    python3 ib_check/pion/vz_figs.py /volatile/clas12/mtenorio/pi0/ib_check/pion

Reads   <dir>/root/pion_ana_<T>_<POL>.root, T = CxC, CuSn, POL = IB, OB
          PionAnalysis/Pi{Plus,Minus}/vz_raw_vs_sector_fine, vz_corr_vs_sector   (x = sector, y = vz, 0.05 cm bins)
          PionAnalysis/Electron/vz_raw_vs_sector_fine, vz_corr_vs_sector        (trigger electron, events with >= 1 pion)
        Correction from data/Vz/vz_corrector_params.txt: electron blocks for the electron; since v1.3 the pion
        blocks (<variant>_pim / _pip) for the pions when the file has them, otherwise the electron ones.
        LD2 is not drawn: no correction is applied there (corrected = raw) and Table 8 has no LD2 row.
Writes  <dir>/vz/vz_fig5p12_<T>_<POL>.png/.pdf   layout of Fig. 5.12: sectors 1-6 and all sectors;
          e- black, pi- blue, pi+ orange; dashed = before, solid = after the correction; each curve normalized
          to unit area. Vertical lines: foil centroids of Table 8 (pi-, pi+) and Table 2 (e-).
        Printed: two-Voigt + pedestal fit (Eq. 8) of the corrected Vz, all sectors, per species:
          mu and sigma_eff (Eq. 9) of each foil, next to Table 8 (pions) and Table 2 (electrons).
Note: the trigger electron already passed its own Vz cut (Table 2 windows) in pion_analysis, so the corrected
      electron curve is clipped at those windows; its fit uses only non-empty bins.
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import numpy as np
import uproot
from scipy.optimize import curve_fit
from scipy.special import voigt_profile

plt.rcParams["font.family"] = "serif"
plt.rcParams["mathtext.fontset"] = "cm"


DIR = Path(sys.argv[1])
OUT = DIR / "vz"
OUT.mkdir(parents=True, exist_ok=True)
TARGETS, POLS = ["CxC", "CuSn"], ["IB", "OB"]
SPECIES = [("Electron", "e-", r"$e^-$", "black"),
           ("PiMinus", "pi-", r"$\pi^-$", "#1f77b4"),
           ("PiPlus", "pi+", r"$\pi^+$", "#ff7f0e")]
# (mu_up, sigma_eff_up, mu_down, sigma_eff_down) [cm]
TABLE = {  # common note Table 2 (e-) and Table 8 (pi-, pi+)
    ("CuSn", "OB", "e-"): (-7.861, 0.415, -2.916, 0.370),
    ("CuSn", "IB", "e-"): (-7.765, 0.321, -2.812, 0.315),
    ("CxC", "OB", "e-"): (-7.887, 0.395, -2.906, 0.373),
    ("CxC", "IB", "e-"): (-7.670, 0.313, -2.713, 0.307),
    ("CuSn", "OB", "pi-"): (-7.777, 0.449, -2.823, 0.382),
    ("CuSn", "OB", "pi+"): (-7.782, 0.403, -2.821, 0.393),
    ("CuSn", "IB", "pi-"): (-7.830, 0.463, -2.921, 0.425),
    ("CuSn", "IB", "pi+"): (-7.840, 0.426, -2.951, 0.401),
    ("CxC", "OB", "pi-"): (-7.815, 0.412, -2.805, 0.389),
    ("CxC", "OB", "pi+"): (-7.783, 0.364, -2.802, 0.409),
    ("CxC", "IB", "pi-"): (-7.744, 0.463, -2.782, 0.437),
    ("CxC", "IB", "pi+"): (-7.799, 0.421, -2.849, 0.410),
}
LINE = {"e-": "-", "pi-": "--", "pi+": ":"}
FIT_LO, FIT_HI = -11.0, 0.0


def find(f, suffix):
    for k in f.keys(recursive=True, cycle=False):
        if k.endswith(suffix):
            return f[k]
    return None


def norm(y, edges):
    s = y.sum() * np.diff(edges)[0]
    return y / s if s > 0 else y


def model(x, a1, m1, s1, g1, a2, m2, s2, g2, c):  # Eq. 8
    return a1 * voigt_profile(x - m1, s1, g1) + a2 * voigt_profile(x - m2, s2, g2) + c


def sigma_eff(s, g):  # Eq. 9: Olivero-Longbothum FWHM / (2 sqrt(2 ln 2))
    fl, fg = 2 * g, 2 * np.sqrt(2 * np.log(2)) * s
    return (0.5346 * fl + np.sqrt(0.2166 * fl**2 + fg**2)) / (2 * np.sqrt(2 * np.log(2)))


def fit_two_voigt(y, yc, start):
    w = 0.05
    sel = (yc >= FIT_LO) & (yc <= FIT_HI) & (y > 0)
    x, yy = yc[sel], y[sel]
    mu1, s1, mu2, s2 = start
    n = yy.sum() * w
    p0 = [0.5 * n, mu1, 0.7 * s1, 0.3 * s1, 0.5 * n, mu2, 0.7 * s2, 0.3 * s2, max(np.median(yy) * 0.1, 1.0)]
    lo = [0, mu1 - 1, 0.01, 0.0, 0, mu2 - 1, 0.01, 0.0, 0]
    hi = [np.inf, mu1 + 1, 2, 2, np.inf, mu2 + 1, 2, 2, np.inf]
    p, _ = curve_fit(model, x, yy, p0=p0, bounds=(lo, hi), sigma=np.sqrt(yy), maxfev=40000)
    chi2 = np.sum(((yy - model(x, *p)) / np.sqrt(yy)) ** 2) / max(len(x) - len(p), 1)
    return (p[1], sigma_eff(p[2], p[3]), p[5], sigma_eff(p[6], p[7])), chi2


rows = []
for t in TARGETS:
    for pol in POLS:
        fn = DIR / "root" / f"pion_ana_{t}_{pol}.root"
        if not fn.exists():
            print("missing", fn)
            continue
        f = uproot.open(fn)
        fig, axs = plt.subplots(2, 4, figsize=(18, 7.5), sharex=True)
        panels = [axs[0, 0], axs[0, 1], axs[0, 2], axs[1, 0], axs[1, 1], axs[1, 2], axs[0, 3]]
        axs[1, 3].axis("off")
        for d, sp, lab, col in SPECIES:
            hr = find(f, f"PionAnalysis/{d}/vz_raw_vs_sector_fine")
            hc = find(f, f"PionAnalysis/{d}/vz_corr_vs_sector")
            if hr is None or hc is None:
                print(f"no v1.1 histograms for {d} in {fn}")
                continue
            vr, vc, ye = hr.values(), hc.values(), hc.axis(1).edges()
            yc = 0.5 * (ye[1:] + ye[:-1])
            for i, ax in enumerate(panels):
                r = vr[i] if i < 6 else vr.sum(axis=0)
                c = vc[i] if i < 6 else vc.sum(axis=0)
                ax.stairs(norm(r, ye), ye, color=col, ls="--", lw=0.8, alpha=0.6)
                ax.stairs(norm(c, ye), ye, color=col, lw=1.2, label=f"{lab} after" if i == 6 else None)
            tab = TABLE[(t, pol, sp)]
            for ax in panels:
                for m in (tab[0], tab[2]):
                    ax.axvline(m, color="gray", ls=LINE[sp], lw=0.8)
            try:
                ours, chi2 = fit_two_voigt(vc.sum(axis=0), yc, tab)
            except (RuntimeError, ValueError):
                ours, chi2 = (np.nan,) * 4, np.nan
            rows.append((t, pol, sp, ours, tab, chi2))
        for i, ax in enumerate(panels):
            ax.set_title("All Sectors" if i == 6 else f"Sector {i + 1}", fontsize=13)
            ax.set_xlim(-11.5, 0)
        for ax in axs[:, 0]:
            ax.set_ylabel("Normalized", fontsize=12)
            ax.yaxis.set_label_coords(-0.08, 0.84)
        for ax in axs[1, :3]:
            ax.set_xlabel(r"$V_z$ [cm]", fontsize=13)
            ax.xaxis.set_label_coords(0.94, -0.08)

        axs[0, 3].set_xlabel(r"$V_z$ [cm]", fontsize=13)
        axs[0, 3].xaxis.set_label_coords(0.94, -0.08)

        axs[0, 3].set_xlabel(r"$V_z$ [cm]")
        axs[0, 3].xaxis.set_label_coords(0.94, -0.08)
        axs[0, 3].tick_params(labelbottom=True)
        """
        handles = [
            Line2D([], [], color="black", ls="--", label=r"$e^-$ before"),
            Line2D([], [], color="black", ls="-",  label=r"$e^-$ after"),

            Line2D([], [], color="#1f77b4", ls="--", label=r"$\pi^-$ before"),
            Line2D([], [], color="#1f77b4", ls="-",  label=r"$\pi^-$ after"),

            Line2D([], [], color="#ff7f0e", ls="--", label=r"$\pi^+$ before"),
            Line2D([], [], color="#ff7f0e", ls="-",  label=r"$\pi^+$ after"),
        ]
        """
        handles = [
            Line2D([], [], color="black", ls="--", label=r"$e^-$ before"),
            Line2D([], [], color="#ff7f0e", ls="--", label=r"$\pi^+$ before"),
            Line2D([], [], color="#1f77b4", ls="--", label=r"$\pi^-$ before"),
            Line2D([], [], color="black", ls="-",  label=r"$e^-$ after"),
            Line2D([], [], color="#1f77b4", ls="-",  label=r"$\pi^-$ after"),
            Line2D([], [], color="#ff7f0e", ls="-",  label=r"$\pi^+$ after"),
        ]

        fig.suptitle(
            f"{t} {pol} $V_z$ : $e^-$ vs. $\pi^+$ and $\pi^-$",
            fontsize=14
        )

        fig.legend(
            handles=handles,
            loc="upper center",
            bbox_to_anchor=(0.87, 0.35),
            ncol=2,
            frameon=False,
            fontsize=12
        )
        
                
        fig.tight_layout()
        for ext in ("png", "pdf"):
            fig.savefig(OUT / f"vz_fig5p12_{t}_{pol}.{ext}", dpi=140, bbox_inches="tight")
        plt.close(fig)
        print("wrote", OUT / f"vz_fig5p12_{t}_{pol}.png")

print("\nEq. 8 fit of the corrected vz (all sectors) vs the note (Table 2 for e-, Table 8 for pions), cm")
print(f"{'target':6} {'pol':3} {'sp':3} | {'mu_up':>7} {'note':>7} | {'seff_up':>7} {'note':>6} | "
      f"{'mu_dn':>7} {'note':>7} | {'seff_dn':>7} {'note':>6} | chi2/ndf")
for t, pol, sp, o, n, chi2 in rows:
    print(f"{t:6} {pol:3} {sp:3} | {o[0]:7.3f} {n[0]:7.3f} | {o[1]:7.3f} {n[1]:6.3f} | "
          f"{o[2]:7.3f} {n[2]:7.3f} | {o[3]:7.3f} {n[3]:6.3f} | {chi2:6.1f}")
