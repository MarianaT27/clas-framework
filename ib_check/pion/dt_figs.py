"""
dt_figs.py (v2) : Delta-t check for charged pions, IB and OB, following the procedure of common note Sec. 5.1.

Usage (from the clas-framework folder):
    python3 ib_check/pion/dt_figs.py /volatile/clas12/mtenorio/pi0/ib_check/pion [NSIG, default 1.5]

Procedure (common note Sec. 5.1, l.639-651):
  1. Delta-t vs p, separately for pi+ and pi-, in the 30 momentum bins of Fig. 5.1 (0.3 to 8.0 GeV).
  2. A Gaussian fit in each bin -> mu_dt, sigma_dt. The note does not state the Delta-t range of these
     fits. Read off Fig. 5.1 (OB LD2 pi+), the fits cover about mu +- 1.5 sigma around the peak, so each
     bin is fitted in mu +- NSIG sigma (default 1.5, second command-line argument), iterated: start at
     the peak with sigma from its FWHM (FWHM / 2.355, so the tails do not inflate the start), fit,
     refit in the new mu +- NSIG sigma (3 passes). Bins with < 100 entries or a failed fit are skipped.
     The fits are also repeated with NSIG = 1.5, 2.0, 2.5 and summarised (dt_nsig_scan.csv).
  3. mu(p) fitted with Eq. 11 (a p^2 + b p + c) and sigma(p) with Eq. 12 (sqrt(s0^2 + (k/p)^2) + c_s p),
     with p = bin centre and the fit errors as weights.
  4. Selection window mu(p) +- 3 sigma(p).
Compared with Table 6 (OB) and Table 7 (IB). For CuSn (both foils in one run) the note gives Cu and Sn
rows separately; both are shown.

Reads   <dir>/root/pion_ana_<T>_<POL>.root   PionAnalysis/Pi{Plus,Minus}/delta_t_vs_p_diag
Writes  <dir>/dt/
        dt_bins_<POL>_<species>_<T>.png/.pdf  Gaussian fits in the 30 bins (Fig. 5.1 style). Display range
                                              of each bin = the axis range of the same bin in Fig. 5.1
                                              (OB LD2 pi+: as read off the figure; other configurations:
                                              the same range in units of sigma around their own peak;
                                              the last two bins, whose Fig. 5.1 fits are degenerate: mu +- 2.6 sigma).
                                              OB LD2 pi+ also shows the fit range of Fig. 5.1 (gray dotted).
        dt_mu_sigma_<POL>.png/.pdf            mu and sigma vs p, this work (points + solid) vs the note (dashed)
        dt_vs_p_<POL>.png/.pdf                Delta-t vs p with the mu +- 3 sigma windows, this work and the note
        dt_bin_fits.csv                       the fit of every bin
        dt_params.csv                         Eq. 11/12 parameters, this work and the note, side by side
        dt_nsig_scan.csv                      median chi2/ndf per configuration for NSIG = 1.5, 2.0, 2.5, and
                                              for OB LD2 pi+ the bin-by-bin mu, sigma, chi2/ndf next to Fig. 5.1
"""
import csv
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import uproot
from matplotlib.colors import LogNorm
from scipy.optimize import curve_fit

DIR = Path(sys.argv[1])
OUT = DIR / "dt"
OUT.mkdir(parents=True, exist_ok=True)
TARGETS = ["LD2", "CxC", "CuSn"]
POLS = ["IB", "OB"]
SPECIES = {"PiPlus": r"$\pi^+$", "PiMinus": r"$\pi^-$"}
COLOR = {"LD2": "black", "CxC": "#1f77b4", "CuSn": "#9467bd"}
# The 30 momentum bins of the common note, Fig. 5.1 (bin labels).
P_EDGES = [0.3, 0.6, 0.8, 1.1, 1.3, 1.6, 1.8, 2.1, 2.4, 2.6, 2.9, 3.1, 3.4, 3.6, 3.9, 4.1,
           4.4, 4.7, 4.9, 5.2, 5.4, 5.7, 5.9, 6.2, 6.5, 6.7, 7.0, 7.2, 7.5, 7.7, 8.0]
NSIG = float(sys.argv[2]) if len(sys.argv) > 2 else 1.5
NSIG_SCAN = [1.5, 2.0, 2.5]
MIN_ENTRIES, PASSES = 100, 3

# Fig. 5.1 (OB, LD2, pi+), read off the figure by eye, one row per bin:
# (mu, sigma, chi2/ndf) printed in the panel, x-axis range (ax_lo, ax_hi), fit range of the red curve (fit_lo, fit_hi) [ns]
FIG51 = [
    (0.13, 0.21, 61.2, -0.400, 0.720, -0.200, 0.500), (0.04, 0.10, 34.3, -0.220, 0.310, -0.120, 0.210),
    (0.03, 0.09, 26.5, -0.200, 0.270, -0.110, 0.180), (0.03, 0.09, 13.0, -0.190, 0.250, -0.100, 0.170),
    (0.03, 0.09, 14.2, -0.180, 0.245, -0.100, 0.170), (0.03, 0.08, 8.1, -0.185, 0.240, -0.100, 0.165),
    (0.03, 0.08, 6.8, -0.180, 0.230, -0.100, 0.160), (0.03, 0.08, 5.6, -0.175, 0.225, -0.100, 0.155),
    (0.03, 0.08, 3.7, -0.170, 0.225, -0.095, 0.155), (0.03, 0.08, 4.0, -0.170, 0.225, -0.095, 0.150),
    (0.03, 0.08, 1.9, -0.165, 0.215, -0.095, 0.150), (0.03, 0.08, 2.1, -0.165, 0.215, -0.090, 0.145),
    (0.02, 0.07, 2.5, -0.165, 0.200, -0.090, 0.135), (0.02, 0.07, 2.9, -0.160, 0.195, -0.095, 0.130),
    (0.02, 0.07, 2.8, -0.155, 0.190, -0.090, 0.125), (0.02, 0.07, 4.3, -0.155, 0.180, -0.085, 0.120),
    (0.02, 0.07, 3.9, -0.155, 0.175, -0.090, 0.115), (0.02, 0.07, 2.8, -0.155, 0.175, -0.085, 0.105),
    (0.01, 0.07, 2.9, -0.150, 0.170, -0.090, 0.110), (0.01, 0.07, 1.6, -0.150, 0.160, -0.090, 0.100),
    (0.01, 0.06, 2.0, -0.145, 0.165, -0.090, 0.100), (0.01, 0.06, 1.8, -0.140, 0.145, -0.090, 0.095),
    (0.01, 0.06, 1.2, -0.135, 0.145, -0.090, 0.095), (0.01, 0.06, 1.1, -0.140, 0.160, -0.085, 0.100),
    (0.01, 0.06, 0.8, -0.130, 0.135, -0.075, 0.085), (0.00, 0.06, 1.3, -0.135, 0.130, -0.090, 0.085),
    (0.01, 0.06, 0.6, -0.120, 0.135, -0.075, 0.090), (0.01, 0.07, 0.7, -0.140, 0.110, -0.095, 0.060),
    (0.00, 0.04, 0.4, -0.005, 0.038, 0.002, 0.030), (-0.04, 0.01, 1.8, -0.053, -0.020, -0.046, -0.025),
]
assert len(FIG51) == len(P_EDGES) - 1

# Common note Table 6 (OB) and Table 7 (IB); same numbers as PionSelectionAlg.hpp.
# (a_mu, b_mu, c_mu), (sigma0, k, c_sigma)
NOTE = {
    ("OB", "PiPlus", "LD2"): ((0.000944, -0.010445, 0.046100), (0.076738, 0.054354, -0.000929)),
    ("OB", "PiPlus", "CxC"): ((0.001103, -0.011081, 0.046219), (0.075224, 0.054430, -0.000447)),
    ("OB", "PiPlus", "Cu"):  ((0.001381, -0.013593, 0.049945), (0.072581, 0.059173, 0.000113)),
    ("OB", "PiPlus", "Sn"):  ((0.001150, -0.010677, 0.041478), (0.076579, 0.050554, -0.000595)),
    ("OB", "PiMinus", "LD2"): ((-0.001640, 0.009816, -0.025958), (0.080510, 0.118046, -0.001205)),
    ("OB", "PiMinus", "CxC"): ((-0.001866, 0.010890, -0.026872), (0.078231, 0.118102, -0.000447)),
    ("OB", "PiMinus", "Cu"):  ((-0.001433, 0.008117, -0.022846), (0.076203, 0.117692, 0.000187)),
    ("OB", "PiMinus", "Sn"):  ((-0.001777, 0.010759, -0.029889), (0.075510, 0.120755, 0.000220)),
    ("IB", "PiPlus", "LD2"): ((-0.001731, 0.011267, -0.054537), (0.085218, 0.120109, -0.003085)),
    ("IB", "PiPlus", "CxC"): ((-0.001542, 0.009979, -0.054565), (0.088016, 0.119062, -0.003361)),
    ("IB", "PiPlus", "Cu"):  ((-0.001792, 0.011443, -0.052059), (0.087525, 0.118368, -0.003076)),
    ("IB", "PiPlus", "Sn"):  ((-0.001860, 0.012422, -0.059442), (0.089254, 0.123236, -0.003298)),
    ("IB", "PiMinus", "LD2"): ((0.000405, -0.006009, 0.004971), (0.076147, 0.054397, -0.001330)),
    ("IB", "PiMinus", "CxC"): ((0.000399, -0.006449, 0.004927), (0.074921, 0.056532, -0.000445)),
    ("IB", "PiMinus", "Cu"):  ((0.000546, -0.007970, 0.012212), (0.078201, 0.054772, -0.001775)),
    ("IB", "PiMinus", "Sn"):  ((0.001120, -0.010184, 0.008613), (0.080358, 0.050856, -0.001380)),
}
PAR_NAMES = ["a_mu", "b_mu", "c_mu", "sigma0", "k", "c_sigma"]


def note_foils(t):
    return ["Cu", "Sn"] if t == "CuSn" else [t]


def mu_f(p, a, b, c):  # Eq. 11
    return a * p**2 + b * p + c


def sig_f(p, s0, k, cs):  # Eq. 12
    return np.sqrt(s0**2 + (k / p) ** 2) + cs * p


def gauss(x, n, m, s):
    return n * np.exp(-0.5 * ((x - m) / s) ** 2)


def find(f, suffix):
    for k in f.keys(recursive=True, cycle=False):
        if k.endswith(suffix):
            return f[k]
    return None


def load(pol, t, sp):
    fn = DIR / "root" / f"pion_ana_{t}_{pol}.root"
    if not fn.exists():
        return None
    h = find(uproot.open(fn), f"PionAnalysis/{sp}/delta_t_vs_p_diag")
    return None if h is None else (h.values(), h.axis(0).edges(), h.axis(1).edges())


def fit_bin(y, yc, nsig):
    """Gaussian fit in mu +- nsig sigma around the peak, iterated PASSES times; start from the FWHM.
    Returns (popt, perr, fit_lo, fit_hi, chi2ndf) or None."""
    if y.sum() < MIN_ENTRIES:
        return None
    ys = np.convolve(y, np.ones(3) / 3, mode="same")  # 3-bin smoothing, only to locate the peak and FWHM
    i0 = int(np.argmax(ys))
    half = ys[i0] / 2
    il, ir = i0, i0
    while il > 0 and ys[il] > half:
        il -= 1
    while ir < len(ys) - 1 and ys[ir] > half:
        ir += 1
    m, s = yc[i0], max((yc[ir] - yc[il]) / 2.355, yc[1] - yc[0])
    popt = perr = None
    for _ in range(PASSES):
        win = np.abs(yc - m) < nsig * s
        if win.sum() < 5:
            return None
        try:
            popt, pcov = curve_fit(gauss, yc[win], y[win], p0=[y[win].max(), m, s],
                                   sigma=np.sqrt(np.maximum(y[win], 1)), absolute_sigma=True, maxfev=10000)
        except RuntimeError:
            return None
        m, s = popt[1], abs(popt[2])
        perr = np.sqrt(np.diag(pcov))
    win = np.abs(yc - m) < nsig * s
    chi2 = np.sum((y[win] - gauss(yc[win], *popt)) ** 2 / np.maximum(y[win], 1)) / max(win.sum() - 3, 1)
    return popt, perr, m - nsig * s, m + nsig * s, chi2


def save(fig, name):
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=140, bbox_inches="tight")
    plt.close(fig)
    print("wrote", OUT / f"{name}.png")


bin_rows, par_rows, scan_rows, ours, points, hists = [], [], [], {}, {}, {}
for pol in POLS:
    for sp in SPECIES:
        for t in TARGETS:
            d = load(pol, t, sp)
            if d is None:
                print(f"missing: {t} {pol} {sp}")
                continue
            hists[(pol, sp, t)] = d
            v, xe, ye = d
            yc = 0.5 * (ye[1:] + ye[:-1])
            fig, axs = plt.subplots(5, 6, figsize=(20, 14))
            pc, mf, me, sf, se = [], [], [], [], []
            for i, (lo, hi) in enumerate(zip(P_EDGES[:-1], P_EDGES[1:])):
                ax = axs.flat[i]
                sel = (xe[:-1] >= lo - 1e-9) & (xe[1:] <= hi + 1e-9)
                y = v[sel].sum(axis=0)
                r = fit_bin(y, yc, NSIG)
                for n_ in NSIG_SCAN:
                    rs = fit_bin(y, yc, n_)
                    scan_rows.append(dict(polarity=pol, species=sp, target=t, p_lo=lo, p_hi=hi, nsig=n_,
                                          mu=rs[0][1] if rs else np.nan, sigma=abs(rs[0][2]) if rs else np.nan,
                                          chi2ndf=rs[4] if rs else np.nan))
                ax.set_title(rf"$p\in$[{lo:.1f}, {hi:.1f}) GeV", fontsize=9)
                ax.tick_params(labelsize=7)
                if r is None:
                    ax.text(0.5, 0.5, f"no fit ({int(y.sum())} entries)", ha="center", transform=ax.transAxes, fontsize=8)
                    continue
                popt, perr, flo, fhi, chi2 = r
                m, s = popt[1], abs(popt[2])
                f51 = FIG51[i]
                if f51[1] < 0.05:  # the last two bins of Fig. 5.1 have degenerate fits (sigma 0.04, 0.01): mu +- 2.6 sigma instead
                    xlo, xhi = m - 2.6 * s, m + 2.6 * s
                elif (pol, sp, t) == ("OB", "PiPlus", "LD2"):
                    xlo, xhi = f51[3], f51[4]
                else:  # same range in units of sigma around this configuration's own peak
                    xlo = m + (f51[3] - f51[0]) / f51[1] * s
                    xhi = m + (f51[4] - f51[0]) / f51[1] * s
                if (pol, sp, t) == ("OB", "PiPlus", "LD2"):
                    for xv in (f51[5], f51[6]):
                        ax.axvline(xv, color="gray", ls=":", lw=1)
                show = (yc >= xlo) & (yc <= xhi)
                ax.errorbar(yc[show], y[show], yerr=np.sqrt(y[show]), fmt="o", ms=1.5, color=COLOR[t], elinewidth=0.5)
                ax.set_xlim(xlo, xhi)
                xx = np.linspace(flo, fhi, 200)
                ax.plot(xx, gauss(xx, *popt), color="red", lw=1.2)
                ax.text(0.97, 0.95, f"$\\mu$={m:.3f} ns\n$\\sigma$={s:.3f} ns\n$\\chi^2_\\nu$={chi2:.1f}",
                        transform=ax.transAxes, ha="right", va="top", fontsize=7,
                        bbox=dict(facecolor="white", edgecolor="gray", lw=0.5))
                if i >= 24:
                    ax.set_xlabel(r"$\Delta t$ [ns]", fontsize=8)
                pm = 0.5 * (lo + hi)
                pc.append(pm); mf.append(m); me.append(perr[1]); sf.append(s); se.append(perr[2])
                bin_rows.append(dict(polarity=pol, species=sp, target=t, p_lo=lo, p_hi=hi, entries=int(y.sum()),
                                     mu=m, mu_err=perr[1], sigma=s, sigma_err=perr[2], fit_lo=flo, fit_hi=fhi, chi2ndf=chi2))
            extra = ", gray dotted = fit range of Fig. 5.1" if (pol, sp, t) == ("OB", "PiPlus", "LD2") else ""
            fig.suptitle(f"{t} {pol}, {SPECIES[sp]}: Gaussian fits in the 30 momentum bins of the common note "
                         f"(fit range $\\mu\\pm{NSIG:g}\\sigma$ of each bin{extra})", fontsize=13)
            fig.tight_layout()
            save(fig, f"dt_bins_{pol}_{sp}_{t}")
            pc, mf, me, sf, se = map(np.array, (pc, mf, me, sf, se))
            points[(pol, sp, t)] = (pc, mf, me, sf, se)
            # Eq. 11 and 12 fits
            try:
                pm_, cm_ = curve_fit(mu_f, pc, mf, sigma=me, absolute_sigma=True)
                ps_, cs_ = curve_fit(sig_f, pc, sf, sigma=se, p0=[0.08, 0.08, 0.0], absolute_sigma=True, maxfev=20000)
            except (RuntimeError, TypeError):
                print(f"Eq. 11/12 fit failed: {t} {pol} {sp}")
                continue
            ours[(pol, sp, t)] = (pm_, ps_)
            errs = np.concatenate([np.sqrt(np.diag(cm_)), np.sqrt(np.diag(cs_))])
            vals = np.concatenate([pm_, ps_])
            row = dict(polarity=pol, species=sp, target=t, source="this work")
            row.update({n: v_ for n, v_ in zip(PAR_NAMES, vals)})
            row.update({n + "_err": e for n, e in zip(PAR_NAMES, errs)})
            par_rows.append(row)
            for foil in note_foils(t):
                (a, b, c), (s0, k, cs) = NOTE[(pol, sp, foil)]
                nrow = dict(polarity=pol, species=sp, target=t, source=f"note Table {6 if pol == 'OB' else 7} ({foil})")
                nrow.update(dict(zip(PAR_NAMES, [a, b, c, s0, k, cs])))
                par_rows.append(nrow)

pgrid = np.linspace(0.35, 8.0, 300)
for pol in POLS:
    tab = 6 if pol == "OB" else 7
    # ---- mu and sigma vs p (Figs. 5.2, 5.7-5.9 style)
    fig, axs = plt.subplots(2, 2, figsize=(12, 8.5))
    for j, sp in enumerate(SPECIES):
        for t in TARGETS:
            if (pol, sp, t) not in points:
                continue
            pc, mf, me, sf, se = points[(pol, sp, t)]
            axs[0, j].errorbar(pc, mf, yerr=me, fmt="o", ms=3, color=COLOR[t], label=f"{t} bins")
            axs[1, j].errorbar(pc, sf, yerr=se, fmt="o", ms=3, color=COLOR[t], label=f"{t} bins")
            if (pol, sp, t) in ours:
                pm_, ps_ = ours[(pol, sp, t)]
                axs[0, j].plot(pgrid, mu_f(pgrid, *pm_), color=COLOR[t], lw=1.3, label=f"{t} Eq. 11 (this work)")
                axs[1, j].plot(pgrid, sig_f(pgrid, *ps_), color=COLOR[t], lw=1.3, label=f"{t} Eq. 12 (this work)")
            for foil, ls in zip(note_foils(t), ["--", ":"]):
                (a, b, c), (s0, k, cs) = NOTE[(pol, sp, foil)]
                axs[0, j].plot(pgrid, mu_f(pgrid, a, b, c), color=COLOR[t], ls=ls, lw=1, label=f"{foil} Table {tab}")
                axs[1, j].plot(pgrid, sig_f(pgrid, s0, k, cs), color=COLOR[t], ls=ls, lw=1, label=f"{foil} Table {tab}")
        axs[0, j].set_title(f"{pol}, {SPECIES[sp]}: centroid")
        axs[1, j].set_title(f"{pol}, {SPECIES[sp]}: resolution")
        axs[0, j].set_ylabel(r"$\mu_{\Delta t}$ [ns]")
        axs[0, j].set_ylim(-0.1, 0.1)
        axs[1, j].set_ylabel(r"$\sigma_{\Delta t}$ [ns]")
        for ax in axs[:, j]:
            ax.set_xlabel(r"$p$ [GeV/$c$]")
            ax.legend(fontsize=6.5, ncol=2)
    fig.suptitle(f"$\\Delta t$ centroid and resolution, {pol}: points and solid = this work, "
                 f"dashed/dotted = common note Table {tab}")
    fig.tight_layout()
    save(fig, f"dt_mu_sigma_{pol}")

    # ---- 2D with both selection windows (Figs. 5.3-5.6 style)
    fig, axs = plt.subplots(len(TARGETS), 2, figsize=(11, 4 * len(TARGETS)), squeeze=False)
    for i, t in enumerate(TARGETS):
        for j, sp in enumerate(SPECIES):
            ax = axs[i, j]
            d = hists.get((pol, sp, t))
            if d is None:
                continue
            v, xe, ye = d
            ax.pcolormesh(xe, ye, np.where(v > 0, v, np.nan).T, norm=LogNorm(), cmap="viridis")
            if (pol, sp, t) in ours:
                pm_, ps_ = ours[(pol, sp, t)]
                m, s = mu_f(pgrid, *pm_), sig_f(pgrid, *ps_)
                ax.plot(pgrid, m, color="red", lw=1.2, label=r"$\mu$ this work")
                ax.plot(pgrid, m + 3 * s, color="red", lw=1.2, label=r"$\mu\pm3\sigma$ this work")
                ax.plot(pgrid, m - 3 * s, color="red", lw=1.2)
            for foil, ls in zip(note_foils(t), ["--", ":"]):
                (a, b, c), (s0, k, cs) = NOTE[(pol, sp, foil)]
                m, s = mu_f(pgrid, a, b, c), sig_f(pgrid, s0, k, cs)
                ax.plot(pgrid, m + 3 * s, color="white", ls=ls, lw=1.1, label=rf"$\mu\pm3\sigma$ Table {tab} ({foil})")
                ax.plot(pgrid, m - 3 * s, color="white", ls=ls, lw=1.1)
            ax.set_ylim(-2, 2)
            ax.set_xlabel(r"$p$ [GeV/$c$]")
            ax.set_ylabel(r"$\Delta t$ [ns]")
            ax.set_title(f"{t} {pol}, {SPECIES[sp]}")
            ax.legend(fontsize=7, loc="upper right", facecolor="lightgray")
    fig.suptitle(f"$\\Delta t$ vs $p$, {pol}: red = this work, white = common note Table {tab}")
    fig.tight_layout()
    save(fig, f"dt_vs_p_{pol}")


def write_csv(name, rows):
    if not rows:
        return
    keys = list(dict.fromkeys(k for r in rows for k in r))
    with open(OUT / name, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=keys)
        w.writeheader()
        w.writerows(rows)
    print("wrote", OUT / name, f"({len(rows)} rows)")


write_csv("dt_bin_fits.csv", bin_rows)
write_csv("dt_params.csv", par_rows)

print(f"\n{'pol':3s} {'species':8s} {'target':6s} {'source':22s}" + "".join(f"{n:>11s}" for n in PAR_NAMES))
for r in par_rows:
    print(f"{r['polarity']:3s} {r['species']:8s} {r['target']:6s} {r['source']:22s}"
          + "".join(f"{r[n]:+11.6f}" for n in PAR_NAMES))

# ---- NSIG scan: median chi2/ndf per configuration, and OB LD2 pi+ bin by bin against Fig. 5.1
print(f"\nmedian chi2/ndf over the bins, per fit range (mu +- NSIG sigma):")
print(f"{'pol':3s} {'species':8s} {'target':6s}" + "".join(f"{'NSIG=' + str(n):>11s}" for n in NSIG_SCAN))
summary = []
for pol in POLS:
    for sp in SPECIES:
        for t in TARGETS:
            meds = []
            for n_ in NSIG_SCAN:
                c = [r["chi2ndf"] for r in scan_rows if (r["polarity"], r["species"], r["target"], r["nsig"]) == (pol, sp, t, n_)]
                c = np.array(c, dtype=float)
                meds.append(np.nanmedian(c) if np.isfinite(c).any() else np.nan)
            if np.isfinite(meds).any():
                print(f"{pol:3s} {sp:8s} {t:6s}" + "".join(f"{v:11.2f}" for v in meds))
                summary.append(dict(polarity=pol, species=sp, target=t, **{f"median_chi2ndf_nsig{n_}": v for n_, v in zip(NSIG_SCAN, meds)}))
write_csv("dt_nsig_scan.csv", scan_rows)

print("\nOB LD2 pi+, bin by bin: Fig. 5.1 (mu, sigma, chi2/ndf) vs this work for each NSIG")
hdr = f"{'p bin':14s} {'Fig5.1':>20s}" + "".join(f"{'NSIG=' + str(n):>22s}" for n in NSIG_SCAN)
print(hdr)
for i, (lo, hi) in enumerate(zip(P_EDGES[:-1], P_EDGES[1:])):
    f = FIG51[i]
    line = f"[{lo:.1f}, {hi:.1f})".ljust(14) + f"{f[0]:7.2f}{f[1]:6.2f}{f[2]:7.1f}"
    for n_ in NSIG_SCAN:
        rr = [r for r in scan_rows if (r["polarity"], r["species"], r["target"], r["nsig"], r["p_lo"]) == ("OB", "PiPlus", "LD2", n_, lo)]
        line += f"{rr[0]['mu']:8.3f}{rr[0]['sigma']:7.3f}{rr[0]['chi2ndf']:7.1f}" if rr else " " * 22
    print(line)
