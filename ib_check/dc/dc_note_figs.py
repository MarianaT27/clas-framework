"""
dc_note_figs.py : electron DC fiducial figures in the format of the common note's Fig. 3.16 (one file per panel),
for IB or OB.

Usage:
    python3 ib_check/dc/dc_note_figs.py <root_dir> <IB|OB> <out_dir>
    e.g. python3 ib_check/dc/dc_note_figs.py /volatile/clas12/mtenorio/pi0/ib_check/root_full IB /volatile/clas12/mtenorio/pi0/ib_check/figs_full/IB/dc_note

Inputs: same as dc_figs.py (<root_dir>/<ib|ob>_fig_{LD2,CxC,CuSn}.root, Tracks/ElectronTracks/edge_vs_chi2ndf_r<R>_S<s>).
Method: same as dc_figs.py (profile <chi2/ndf> vs edge, 5-point moving variance, threshold 0.05, scan from 20 cm for
        R1/R2 and 30 cm for R3); the cuts are identical to dc_figs.py / table3_dc_cuts_<pol>.csv.
Style:  as the note's sector_comparison_r<R>.pdf: title "Region <R> - Sector Comparison", y "chi2/ndf" (top),
        x "Edge [cm]" (right), 0-25 cm, ticks inside, dotted grid, markers joined by lines with error bars,
        "Cut values:" box, dashed cut line per sector; Computer Modern / Latin Modern serif fonts. No OB reference line.
Writes (per target T):
  <out_dir>/<T>_<POL>/sector_comparison/sector_comparison_r1.pdf/.png, _r2, _r3     panels (b)-(d)
  <out_dir>/<T>_<POL>/fiducial_cut_r1_S1.pdf/.png                                   panel (a): R1 S1 profile with the
                                                                                     cut, as the note's panel (a)
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib import font_manager
import numpy as np
import uproot

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from common import args, fname, SECTOR_COLORS

ROOT, POL, OUT = args()
TARGETS = ["LD2", "CxC", "CuSn"]
WIN, VAR_MAX, MIN_N = 5, 0.05, 1
EMAX = {1: 20.0, 2: 20.0, 3: 30.0}
XMAX = 25.0

# ---- typography: Latin Modern if installed, else Computer Modern (matplotlib's cmr10), math in Computer Modern
_avail = {f.name for f in font_manager.fontManager.ttflist}
_serif = [f for f in ("Latin Modern Roman", "LM Roman 10", "CMU Serif", "cmr10") if f in _avail] or ["DejaVu Serif"]
plt.rcParams.update({
    "font.family": "serif", "font.serif": _serif, "mathtext.fontset": "cm", "axes.formatter.use_mathtext": True,
    "font.size": 14, "axes.titlesize": 16, "axes.labelsize": 15, "legend.fontsize": 13,
    "xtick.direction": "in", "ytick.direction": "in", "xtick.top": True, "ytick.right": True,
    "xtick.minor.visible": True, "ytick.minor.visible": True,
    "xtick.major.size": 6, "ytick.major.size": 6, "xtick.minor.size": 3, "ytick.minor.size": 3,
    "axes.grid": True, "grid.linestyle": ":", "grid.alpha": 0.5, "grid.linewidth": 0.6,
    "axes.unicode_minus": False,
})
FIGSIZE = (11.62, 6.62)  # size of the note's sector_comparison_r1.pdf (836.6 x 476.6 pt)
FIGSIZE_A = (12.0, 5.2)  # panel (a): wide single panel, as the note's R1 S1 profile


def profile_2d(v, ex, ey):
    xc, yc = 0.5 * (ex[1:] + ex[:-1]), 0.5 * (ey[1:] + ey[:-1])
    n = v.sum(axis=1)
    with np.errstate(invalid="ignore", divide="ignore"):
        m = (v * yc).sum(axis=1) / n
        rms = np.sqrt(np.clip((v * yc**2).sum(axis=1) / n - m**2, 0, None))
        e = rms / np.sqrt(n)
    return xc, m, e, n


def sector_profile(f, r, s):
    h = f[f"Tracks/ElectronTracks/edge_vs_chi2ndf_r{r}_S{s}"]
    return profile_2d(h.values(), h.axis(0).edges(), h.axis(1).edges())


def moving_variance(x, m, n, r):
    ok = (n >= MIN_N) & (x < EMAX[r]) & np.isfinite(m)
    xs, ms = x[ok], m[ok]
    var = np.full(len(xs), np.nan)
    cut = np.nan
    for i in range(len(xs) - WIN, -1, -1):
        var[i + WIN // 2] = np.var(ms[i:i + WIN])
        if np.isnan(cut) and var[i + WIN // 2] > VAR_MAX:
            cut = xs[i + WIN // 2]
    return cut, xs, var


def axes_labels(ax, ylabel=r"$\chi^2$/ndf"):
    ax.set_xlim(0, XMAX)
    ax.set_xlabel("Edge [cm]", loc="right")
    ax.set_ylabel(ylabel, loc="top")


def save(fig, out, name):
    out.mkdir(parents=True, exist_ok=True)
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(out / f"{name}.{ext}", dpi=150)
    plt.close(fig)
    print("wrote", out / f"{name}.pdf")


def plot_profile(ax, prof, color, label):
    x, m, e, n = prof
    ok = (n > 0) & (x <= XMAX)
    ax.errorbar(x[ok], m[ok], yerr=e[ok], fmt="o-", ms=3.5, lw=1.0, elinewidth=0.8, capsize=1.5,
                color=color, label=label)


for t in TARGETS:
    f = uproot.open(fname(ROOT, POL, t))
    base = OUT / f"{t}_{POL}"

    # panels (b)-(d): sector comparison per region
    for r in (1, 2, 3):
        fig, ax = plt.subplots(figsize=FIGSIZE)
        lines = []
        for s in range(1, 7):
            prof = sector_profile(f, r, s)
            c = moving_variance(prof[0], prof[1], prof[3], r)[0]
            plot_profile(ax, prof, SECTOR_COLORS[s - 1], f"Sector {s}")
            if np.isfinite(c):
                ax.axvline(c, color=SECTOR_COLORS[s - 1], ls="--", lw=1.5)
            lines.append(f"S{s}: {c:.2f} cm" if np.isfinite(c) else f"S{s}: none")
        ax.text(0.015, 0.975, "Cut values:\n" + "\n".join(lines), transform=ax.transAxes, va="top", ha="left",
                fontsize=12, linespacing=1.15, bbox=dict(boxstyle="square,pad=0.3", fc="white", ec="black", lw=1))
        ax.set_title(f"Region {r} - Sector Comparison")
        axes_labels(ax)
        ax.legend(loc="upper right", frameon=False)
        save(fig, base / "sector_comparison", f"sector_comparison_r{r}")

    # panel (a): R1 S1 profile with the cut, as the note's fiducial_cut_..._r1_S1_1.pdf
    x, m, e, n = sector_profile(f, 1, 1)
    cut = moving_variance(x, m, n, 1)[0]
    ok = (n > 0) & (x <= XMAX)
    fig, ax = plt.subplots(figsize=FIGSIZE_A)
    ax.errorbar(x[ok], m[ok], yerr=e[ok], fmt="o", ms=4, lw=0.8, capsize=1.5, color="#3b5bbd", label="Data")
    if np.isfinite(cut):
        ax.axvline(cut, color="#00c853", ls="--", lw=2, label="Fiducial Cut")
        ax.text(0.60, 0.94, f"Cut at {cut:.2f} cm\nMoving Variance Method", transform=ax.transAxes, va="top",
                ha="left", fontsize=12, bbox=dict(boxstyle="square,pad=0.3", fc="white", ec="black", lw=1))
    ax.set_title(r"$\chi^2$/ndf vs Edge - Region 1, Sector 1")
    axes_labels(ax)
    ax.legend(loc="upper right", frameon=False, fontsize=10)
    save(fig, base, "fiducial_cut_r1_S1")


# ---- appendix figures (v2): momentum bins, theta bins (LD2) and target comparison (sector 1), one PDF per region,
# ---- in <out_dir>/dc_appendix/. Momentum and theta: DC/chi2ndf_vs_edge_vs_{p,theta}_r<R> (full-track chi2/ndf, all
# ---- sectors), histogram slices merged into the note's bins (1 GeV from 1 to 10 GeV; 5 deg from 5 to 35 deg) by
# ---- slice centre. Targets: same per-sector histograms as the sector comparison, sector 1.
APP = OUT / "dc_appendix"
APP.mkdir(parents=True, exist_ok=True)
TARGET_STYLE = {"LD2": ("black", "o"), "CxC": ("#1f77b4", "s"), "CuSn": ("#9467bd", "^")}


def save_pdf(fig, name):
    fig.tight_layout()
    fig.savefig(APP / f"{name}.pdf")
    plt.close(fig)
    print("wrote", APP / f"{name}.pdf")


def merged_profiles(f, kind, r, bins):
    h = f[f"DC/chi2ndf_vs_edge_vs_{kind}_r{r}"]
    v, e0, e1, e2 = h.values(), h.axis(0).edges(), h.axis(1).edges(), h.axis(2).edges()
    c0 = 0.5 * (e0[1:] + e0[:-1])
    out = []
    for lo, hi in bins:
        sel = (c0 >= lo) & (c0 < hi)
        out.append(((lo, hi), *profile_2d(v[sel].sum(axis=0), e1, e2)))
    return out


def cut_box_app(ax, lines):
    ax.text(0.015, 0.975, "Cut values:\n" + "\n".join(lines), transform=ax.transAxes, va="top", ha="left",
            fontsize=12, linespacing=1.15, bbox=dict(boxstyle="square,pad=0.3", fc="white", ec="black", lw=1))


f_ld2 = uproot.open(fname(ROOT, POL, "LD2"))
for kind, bins, cmap, lab_fn, title in (
        ("p", [(k, k + 1) for k in range(1, 10)], "viridis", lambda lo, hi: f"{lo:g}-{hi:g} GeV", "Momentum Comparison"),
        ("theta", [(k, k + 5) for k in range(5, 35, 5)], "plasma",
         lambda lo, hi: rf"$\theta$: {lo:g}-{hi:g}$^\circ$", r"$\theta$ Comparison")):
    for r in (1, 2, 3):
        fig, ax = plt.subplots(figsize=FIGSIZE)
        lines = []
        profs = merged_profiles(f_ld2, kind, r, bins)
        for k, ((lo, hi), x, m, e, n) in enumerate(profs):
            col = plt.get_cmap(cmap)(0.9 * k / max(len(profs) - 1, 1))
            lab = lab_fn(lo, hi)
            if n.sum() == 0:
                lines.append(f"{lab}: no entries")
                continue
            c = moving_variance(x, m, n, r)[0]
            plot_profile(ax, (x, m, e, n), col, lab)
            if np.isfinite(c):
                ax.axvline(c, color=col, ls="--", lw=1.5)
            lines.append(f"{lab}: {c:.2f} cm" if np.isfinite(c) else f"{lab}: none")
        cut_box_app(ax, lines)
        ax.set_title(rf"$\chi^2$/ndf vs Edge - Region {r} ({title})")
        axes_labels(ax)
        ax.legend(loc="upper right", frameon=False, ncol=2)
        save_pdf(fig, f"{'momentum' if kind == 'p' else 'theta'}_comparison_r{r}")

files = {t: uproot.open(fname(ROOT, POL, t)) for t in TARGETS}
for r in (1, 2, 3):
    fig, ax = plt.subplots(figsize=FIGSIZE)
    lines = []
    for t in TARGETS:
        col, mk = TARGET_STYLE[t]
        x, m, e, n = sector_profile(files[t], r, 1)
        c = moving_variance(x, m, n, r)[0]
        ok = (n > 0) & (x <= XMAX)
        ax.errorbar(x[ok], m[ok], yerr=e[ok], fmt=f"{mk}-", ms=3.5, lw=1.0, elinewidth=0.8, capsize=1.5,
                    color=col, label=t)
        if np.isfinite(c):
            ax.axvline(c, color=col, ls="--", lw=1.5)
        lines.append(f"{t}: {c:.2f} cm" if np.isfinite(c) else f"{t}: none")
    cut_box_app(ax, lines)
    ax.set_title(rf"$\chi^2$/ndf vs Edge - Region {r}, Sector 1 ({POL})")
    axes_labels(ax)
    ax.legend(loc="upper right", frameon=False)
    save_pdf(fig, f"target_comparison_r{r}_S1")
