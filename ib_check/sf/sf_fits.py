"""
sf_fits.py : IB versions of the common-note Figs. 3.6, 3.7 and 3.8 (SF vs p fits).

Usage (from the clas-framework folder, where fid_OB.root is):
    python3 /volatile/clas12/mtenorio/pi0/ib_check/sf/sf_fits.py /volatile/clas12/mtenorio/pi0/ib_check/root fid_OB.root /volatile/clas12/mtenorio/pi0/ib_check/sf

Method (note Sec. 3.1.1): SF vs p of trigger electrons (ElectronTracks/p_vs_SF_S<s>), cut into 50 momentum
slices from 0 to 11 GeV; in each slice with enough entries, a Gaussian is fitted around the peak
(iterated twice, +-2 sigma) giving mu_SF and sigma_SF; then mu(p) and sigma(p) are fitted with
a + b p + c p^2 + d p^3 over the momentum range where slices were fitted.

Reads:  <root_dir>/ib_fig_{LD2,CxC,CuSn}.root and <ob_ld2_file> (LD2 OB, same histograms)
Writes: <out_dir>/fig3_6_sf_sectors_LD2.png/.pdf   mu(p), sigma(p) per sector, LD2 IB: fit (solid) vs code IB parameters (dashed)
        <out_dir>/fig3_7_sf_targets.png/.pdf       sector-averaged mu(p), sigma(p), LD2/CxC/CuSn IB, and the code IB average
        <out_dir>/fig3_8_sf_ib_ob_LD2.png/.pdf     sector-averaged mu(p), sigma(p), LD2 IB vs LD2 OB
        <out_dir>/sf_fit_params_IB.csv             fitted a,b,c,d for mu and sigma, per target and sector
Prints: per target and sector, the largest |mu_fit - mu_code| / sigma_code between 2 and 9 GeV.
"Sector-averaged" = mean of the six sector curves.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

if len(sys.argv) != 4:
    sys.exit("usage: python3 sf_fits.py <root_dir> <ob_ld2_file> <out_dir>")
ROOT, OBFILE, OUT = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
OUT.mkdir(parents=True, exist_ok=True)

TARGETS = {"LD2": ("D (LD2)", "black"), "CxC": ("C (CxC)", "#1f77b4"), "CuSn": ("CuSn", "#9467bd")}
SECTOR_COLORS = plt.get_cmap("tab10").colors[:6]
N_SLICES, P_MAX, MIN_ENTRIES = 50, 11.0, 200
P_CMP = (2.0, 9.0)

# SamplingFractionService.hpp, m_data["IB"] (= note Table 14, "Average")
CODE = {
 "IB": {"mu": [[2.311088E-01, 4.309933E-03, -5.451627E-04, 4.965823E-06],
               [2.304337E-01, 3.585716E-03, -1.672223E-04, -7.317175E-06],
               [2.269123E-01, 8.797582E-03, -1.171416E-03, 3.051342E-05],
               [2.288870E-01, 6.786520E-03, -8.022602E-04, 1.557251E-05],
               [2.288645E-01, 4.637362E-03, -3.662892E-04, -4.233509E-06],
               [2.272093E-01, 6.581981E-03, -8.197177E-04, 1.632178E-05]],
        "sig": [[2.235239E-02, -3.117249E-03, 2.887205E-04, -1.130317E-05],
                [2.170118E-02, -2.428037E-03, 1.302147E-04, -2.503248E-09],
                [2.423467E-02, -4.301928E-03, 4.742880E-04, -1.983022E-05],
                [2.313626E-02, -3.260271E-03, 3.018634E-04, -1.156770E-05],
                [2.310895E-02, -3.892876E-03, 4.115173E-04, -1.656769E-05],
                [2.321748E-02, -3.665685E-03, 3.804588E-04, -1.587616E-05]]},
}


def cubic(p, a, b, c, d):
    return a + b * p + c * p**2 + d * p**3


def gauss(x, n, mu, s):
    return n * np.exp(-0.5 * ((x - mu) / s) ** 2)


def slice_fits(f, sector):
    """Gaussian fits of SF in momentum slices -> arrays p, mu, emu, sig, esig."""
    h = f[f"Tracks/ElectronTracks/p_vs_SF_S{sector}"]
    v, ep, es = h.values(), h.axis(0).edges(), h.axis(1).edges()
    sc = 0.5 * (es[1:] + es[:-1])
    pc = 0.5 * (ep[1:] + ep[:-1])
    out = []
    for lo, hi in zip(np.linspace(0, P_MAX, N_SLICES + 1)[:-1], np.linspace(0, P_MAX, N_SLICES + 1)[1:]):
        sel = (pc >= lo) & (pc < hi)
        y = v[sel].sum(axis=0)
        if y.sum() < MIN_ENTRIES:
            continue
        mu, s = sc[np.argmax(y)], 0.02
        try:
            for _ in range(2):
                w = (sc > mu - 2 * s) & (sc < mu + 2 * s) & (y > 0)
                if w.sum() < 5:
                    raise RuntimeError
                popt, pcov = curve_fit(gauss, sc[w], y[w], p0=[y[w].max(), mu, s], sigma=np.sqrt(y[w]))
                mu, s = popt[1], abs(popt[2])
            err = np.sqrt(np.diag(pcov))
            out.append((0.5 * (lo + hi), mu, err[1], s, err[2]))
        except (RuntimeError, ValueError):
            continue
    return np.array(out)


def fit_curves(f):
    """Per sector: slice points and cubic parameters for mu and sigma."""
    res = {}
    for s in range(1, 7):
        pts = slice_fits(f, s)
        if len(pts) < 5:
            sys.exit(f"{f.file_path}: sector {s} has only {len(pts)} momentum slices with >= {MIN_ENTRIES} "
                     f"entries, too few to fit. More files are needed for this sample.")
        pmu, _ = curve_fit(cubic, pts[:, 0], pts[:, 1], sigma=pts[:, 2])
        psg, _ = curve_fit(cubic, pts[:, 0], pts[:, 3], sigma=pts[:, 4])
        res[s] = (pts, pmu, psg)
    return res


fits = {t: fit_curves(uproot.open(ROOT / f"ib_fig_{t}.root")) for t in TARGETS}
fit_ob = fit_curves(uproot.open(OBFILE))
pp = np.linspace(1.0, 10.0, 300)

# ---- printout and csv
print(f"=== IB fits vs code IB parameters: max |mu_fit - mu_code| / sigma_code for {P_CMP[0]} < p < {P_CMP[1]} GeV")
rows = ["target,sector,a_mu,b_mu,c_mu,d_mu,a_sig,b_sig,c_sig,d_sig,n_slices,p_min,p_max"]
pc = np.linspace(*P_CMP, 200)
for t in TARGETS:
    line = []
    for s in range(1, 7):
        pts, pmu, psg = fits[t][s]
        cm, cs = cubic(pc, *CODE["IB"]["mu"][s - 1]), cubic(pc, *CODE["IB"]["sig"][s - 1])
        line.append(f"S{s} {np.max(np.abs(cubic(pc, *pmu) - cm) / cs):.2f}")
        rows.append(",".join([t, str(s)] + [f"{x:.6E}" for x in (*pmu, *psg)]
                             + [str(len(pts)), f"{pts[0, 0]:.2f}", f"{pts[-1, 0]:.2f}"]))
    print(f"    {t:5s} " + "   ".join(line))
(OUT / "sf_fit_params_IB.csv").write_text("\n".join(rows) + "\n")
print(f"wrote {OUT / 'sf_fit_params_IB.csv'}")


def save(fig, name):
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=140)
    plt.close(fig)
    print(f"wrote {OUT / name}.png/.pdf")


def avg(res, which):
    return np.mean([cubic(pp, *res[s][1 if which == "mu" else 2]) for s in range(1, 7)], axis=0)


# ---- Fig. 3.6: per sector, LD2 IB
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
for s in range(1, 7):
    pts, pmu, psg = fits["LD2"][s]
    c = SECTOR_COLORS[s - 1]
    axs[0].errorbar(pts[:, 0], pts[:, 1], yerr=pts[:, 2], fmt="o", ms=2, color=c, alpha=0.5)
    axs[0].plot(pp, cubic(pp, *pmu), color=c, label=f"Sector {s}")
    axs[0].plot(pp, cubic(pp, *CODE["IB"]["mu"][s - 1]), color=c, ls="--", lw=0.8)
    axs[1].errorbar(pts[:, 0], pts[:, 3], yerr=pts[:, 4], fmt="o", ms=2, color=c, alpha=0.5)
    axs[1].plot(pp, cubic(pp, *psg), color=c)
    axs[1].plot(pp, cubic(pp, *CODE["IB"]["sig"][s - 1]), color=c, ls="--", lw=0.8)
axs[0].set_ylabel(r"$\mu$ ($E_{dep,tot}/p$)")
axs[1].set_ylabel(r"$\sigma$ ($E_{dep,tot}/p$)")
axs[1].set_xlabel(r"$p$ [GeV]")
axs[0].legend(fontsize=8, ncol=3, frameon=False)
axs[0].set_title("D (LD2) IB run 18335, per sector: fit (solid, points) vs code IB parameters (dashed)", fontsize=9)
save(fig, "fig3_6_sf_sectors_LD2")

# ---- Fig. 3.7: sector-averaged, targets
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
code_avg_mu = np.mean([cubic(pp, *c) for c in CODE["IB"]["mu"]], axis=0)
code_avg_sg = np.mean([cubic(pp, *c) for c in CODE["IB"]["sig"]], axis=0)
for t, (lab, col) in TARGETS.items():
    axs[0].plot(pp, avg(fits[t], "mu"), color=col, label=f"{lab} IB")
    axs[1].plot(pp, avg(fits[t], "sig"), color=col)
axs[0].plot(pp, code_avg_mu, color="gray", ls="--", label="code IB")
axs[1].plot(pp, code_avg_sg, color="gray", ls="--")
axs[0].set_ylabel(r"$\mu$ ($E_{dep,tot}/p$)")
axs[1].set_ylabel(r"$\sigma$ ($E_{dep,tot}/p$)")
axs[1].set_xlabel(r"$p$ [GeV]")
axs[0].legend(fontsize=8, frameon=False)
axs[0].set_title("Sector-averaged SF parameters, IB: targets vs code", fontsize=9)
save(fig, "fig3_7_sf_targets")

# ---- Fig. 3.8: LD2 IB vs OB
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
for res, lab, col in ((fit_ob, "Outbending", "tab:blue"), (fits["LD2"], "Inbending", "tab:red")):
    axs[0].plot(pp, avg(res, "mu"), color=col, label=lab)
    axs[1].plot(pp, avg(res, "sig"), color=col)
axs[0].set_ylabel(r"$\mu$ ($E_{dep,tot}/p$)")
axs[1].set_ylabel(r"$\sigma$ ($E_{dep,tot}/p$)")
axs[1].set_xlabel(r"$p$ [GeV]")
axs[0].legend(fontsize=8, frameon=False)
axs[0].set_title(f"LD2: IB (run 18335) vs OB ({OBFILE.name}), sector-averaged", fontsize=9)
save(fig, "fig3_8_sf_ib_ob_LD2")
