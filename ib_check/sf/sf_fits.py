"""
sf_fits.py : common-note Figs. 3.6, 3.7 and 3.8 (SF vs p fits), for IB or OB, with the PCAL fiducial cut applied.

Usage:
    python3 sf_fits.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 sf_fits.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/sf

Sample: SF vs p of trigger electrons with PCAL v > 9 cm and w > 9 cm (Tracks/ElectronTracks/p_vs_SF_pcalfid_S<s>).
Method (note Sec. 3.1.1): 50 momentum slices from 0 to 11 GeV; in each slice with at least 200 entries, a Gaussian is
fitted in a fixed symmetric window (code mean of the polarity +- 0.02, re-centred once on the fitted mean;
see common.sf_peak_fit) giving mu_SF and sigma_SF; then mu(p) and sigma(p)
are fitted with a + b p + c p^2 + d p^3, using only the slices with P_FIT[0] <= p <= P_FIT[1] (1.8 to 9.0 GeV);
outside that range mu(p) and sigma(p) are extrapolated from the cubic (dotted in the figures).

Reads:  <root_dir>/<pol>_fig_{LD2,CxC,CuSn}.root, and <root_dir>/{ib,ob}_fig_LD2.root for Fig. 3.8
Writes: <out_dir>/fig3_6_sf_sectors_LD2.png/.pdf   mu(p), sigma(p) per sector, LD2: fit (solid, points) vs code parameters (dashed)
        <out_dir>/fig3_7_sf_targets.png/.pdf       sector-averaged mu(p), sigma(p), LD2/CxC/CuSn, and the code average
        <out_dir>/fig3_8_sf_ib_ob_LD2.png/.pdf     sector-averaged mu(p), sigma(p), LD2 IB vs LD2 OB (same in both runs)
        <out_dir>/sf_fit_params_<pol>.csv          fitted a,b,c,d for mu and sigma, per target and sector
Prints: per target and sector, the largest |mu_fit - mu_code| / sigma_code between 2 and 9 GeV.
"Sector-averaged" = mean of the six sector curves.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import (args, fname, run_of, NAMES, COLORS, SECTOR_COLORS, CODE_SF, cubic, save,
                    code_band, sf_peak_fit, SF_HALF_WINDOW)

ROOT, POL, OUT = args()
TARGETS = ["LD2", "CxC", "CuSn"]
HIST = "Tracks/ElectronTracks/p_vs_SF_pcalfid_S{}"
N_SLICES, P_MAX, MIN_ENTRIES = 50, 11.0, 200
# Momentum range of the slices used in the cubic fits of mu(p) and sigma(p). Outside it the cubic is an
# extrapolation (dotted in the figures); slices outside it are still fitted and drawn in gray.
P_FIT = (1.8, 9.0)
P_CMP = (2.0, 9.0)


def slice_fits(f, sector):
    """Gaussian fits of SF in momentum slices -> rows (p, mu, emu, sig, esig)."""
    h = f[HIST.format(sector)]
    v, ep, es = h.values(), h.axis(0).edges(), h.axis(1).edges()
    sc = 0.5 * (es[1:] + es[:-1])
    pc = 0.5 * (ep[1:] + ep[:-1])
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


def fit_curves(path):
    """Per sector: slice points and cubic parameters for mu and sigma."""
    f = uproot.open(path)
    res = {}
    for s in range(1, 7):
        allpts = slice_fits(f, s)
        pts = allpts[(allpts[:, 0] >= P_FIT[0]) & (allpts[:, 0] <= P_FIT[1])] if len(allpts) else allpts
        if len(pts) < 5:
            sys.exit(f"{path}: sector {s} has only {len(pts)} momentum slices with >= {MIN_ENTRIES} entries "
                     f"in {P_FIT[0]} < p < {P_FIT[1]} GeV.")
        pmu, _ = curve_fit(cubic, pts[:, 0], pts[:, 1], sigma=pts[:, 2])
        psg, _ = curve_fit(cubic, pts[:, 0], pts[:, 3], sigma=pts[:, 4])
        res[s] = (pts, pmu, psg, allpts)
    return res


fits = {t: fit_curves(fname(ROOT, POL, t)) for t in TARGETS}
other = "OB" if POL == "IB" else "IB"
fits_ld2 = {POL: fits["LD2"], other: fit_curves(fname(ROOT, other, "LD2"))}
pp = np.linspace(1.0, 10.0, 300)

# ---- printout and csv
print(f"=== {POL} fits vs code {POL} parameters: max |mu_fit - mu_code| / sigma_code for {P_CMP[0]} < p < {P_CMP[1]} GeV")
rows = ["target,run,sector,a_mu,b_mu,c_mu,d_mu,a_sig,b_sig,c_sig,d_sig,n_slices,p_min,p_max"]
pc = np.linspace(*P_CMP, 200)
for t in TARGETS:
    line = []
    for s in range(1, 7):
        pts, pmu, psg, _ = fits[t][s]
        cm, cs = cubic(pc, *CODE_SF[POL]["mu"][s - 1]), cubic(pc, *CODE_SF[POL]["sig"][s - 1])
        line.append(f"S{s} {np.max(np.abs(cubic(pc, *pmu) - cm) / cs):.2f}")
        rows.append(",".join([t, str(run_of(POL, t)), str(s)] + [f"{x:.6E}" for x in (*pmu, *psg)]
                             + [str(len(pts)), f"{pts[0, 0]:.2f}", f"{pts[-1, 0]:.2f}"]))
    print(f"    {t:5s} " + "   ".join(line))
(OUT / f"sf_fit_params_{POL}.csv").write_text("\n".join(rows) + "\n")
print(f"wrote {OUT / f'sf_fit_params_{POL}.csv'}")


def avg(res, which):
    return np.mean([cubic(pp, *res[s][1 if which == "mu" else 2]) for s in range(1, 7)], axis=0)


IN_FIT = (pp >= P_FIT[0]) & (pp <= P_FIT[1])


def curve(ax, y, color, label=None, lw=1.5):
    """Solid inside the fit range, dotted where the cubic is extrapolated."""
    ax.plot(pp, np.where(IN_FIT, y, np.nan), color=color, lw=lw, label=label)
    ax.plot(pp, np.where(pp <= P_FIT[0], y, np.nan), color=color, lw=lw, ls=":")
    ax.plot(pp, np.where(pp >= P_FIT[1], y, np.nan), color=color, lw=lw, ls=":")


def labels(axs, title):
    axs[0].set_ylabel(r"$\mu$ ($E_{dep,tot}/p$)")
    axs[1].set_ylabel(r"$\sigma$ ($E_{dep,tot}/p$)")
    axs[1].set_xlabel(r"$p$ [GeV]")
    axs[0].legend(fontsize=8, ncol=3, frameon=False)
    axs[0].set_title(title, fontsize=9)


# ---- Fig. 3.6: per sector, LD2
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
for s in range(1, 7):
    pts, pmu, psg, allpts = fits["LD2"][s]
    c = SECTOR_COLORS[s - 1]
    out_rng = (allpts[:, 0] < P_FIT[0]) | (allpts[:, 0] > P_FIT[1])
    for k, (iv, ie) in enumerate(((1, 2), (3, 4))):
        axs[k].errorbar(pts[:, 0], pts[:, iv], yerr=pts[:, ie], fmt="o", ms=2, color=c, alpha=0.5)
        axs[k].errorbar(allpts[out_rng, 0], allpts[out_rng, iv], yerr=allpts[out_rng, ie], fmt="o", ms=2,
                        color="gray", alpha=0.4)
    curve(axs[0], cubic(pp, *pmu), c, f"Sector {s}")
    axs[0].plot(pp, cubic(pp, *CODE_SF[POL]["mu"][s - 1]), color=c, ls="--", lw=0.8)
    curve(axs[1], cubic(pp, *psg), c)
    axs[1].plot(pp, cubic(pp, *CODE_SF[POL]["sig"][s - 1]), color=c, ls="--", lw=0.8)
for ax in axs:
    ax.axvspan(P_FIT[0], P_FIT[1], color="0.9", zorder=0)
axs[0].set_ylim(0.16, 0.26)
axs[1].set_ylim(0.0, 0.05)
labels(axs, f"{NAMES['LD2']} {POL} run {run_of(POL, 'LD2')}, PCAL fiducial, per sector: "
            f"fit (solid), extrapolation (dotted), code {POL} (dashed)")
save(fig, OUT, "fig3_6_sf_sectors_LD2")

# ---- Fig. 3.7: sector-averaged, targets
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
for t in TARGETS:
    curve(axs[0], avg(fits[t], "mu"), COLORS[t], f"{NAMES[t]}, run {run_of(POL, t)}")
    curve(axs[1], avg(fits[t], "sig"), COLORS[t])
axs[0].plot(pp, np.mean([cubic(pp, *c) for c in CODE_SF[POL]["mu"]], axis=0), color="gray", ls="--", label=f"code {POL}")
axs[1].plot(pp, np.mean([cubic(pp, *c) for c in CODE_SF[POL]["sig"]], axis=0), color="gray", ls="--")
labels(axs, f"Sector-averaged SF parameters, {POL}, PCAL fiducial: targets vs code")
save(fig, OUT, "fig3_7_sf_targets")

# ---- Fig. 3.8: LD2 IB vs OB
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
for pol, col in (("OB", "tab:blue"), ("IB", "tab:red")):
    name = "Outbending" if pol == "OB" else "Inbending"
    curve(axs[0], avg(fits_ld2[pol], "mu"), col, f"{name}, run {run_of(pol, 'LD2')}")
    curve(axs[1], avg(fits_ld2[pol], "sig"), col)
labels(axs, "LD2: IB vs OB, PCAL fiducial, sector-averaged")
save(fig, OUT, "fig3_8_sf_ib_ob_LD2")
