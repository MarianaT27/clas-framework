"""
dc_figs.py : IB versions of the common-note Figs. 3.15 to 3.19 and Table 3 (DC fiducial cut on edge).  Version 4.

Usage (from the clas-framework folder, where fid_OB.root is):
    python3 /volatile/clas12/mtenorio/pi0/ib_check/dc/dc_figs.py /volatile/clas12/mtenorio/pi0/ib_check/root fid_OB.root /volatile/clas12/mtenorio/pi0/ib_check/dc

Inputs:
  <root_dir>/ib_fig_{LD2,CxC,CuSn}.root and <ob_ld2_file> (LD2 OB, Tuesday's fid_OB.root), all from electron_analysis
  with the PCAL and DC cuts off.
  Per sector : Tracks/ElectronTracks/edge_vs_chi2ndf_r<R>_S<s>  (trigger electrons, before cuts; chi2/ndf of the
               region's track segment; edge 0-50 cm in 0.25 cm bins, chi2/ndf 0-10)
  p and theta: DC/chi2ndf_vs_edge_vs_p_r<R>, DC/chi2ndf_vs_edge_vs_theta_r<R>  (full-track chi2/ndf, all sectors)

Plots show every edge bin with at least one entry, from 0 to 25 cm, with error bars (RMS / sqrt(N)).
Every profile also gets its own cut from the moving variance, drawn as a dashed line in the profile's colour and listed
in a box ("Cut values"), as in the note; the note's OB cut (1.88 / 2.08 / 8.62 cm) is the black dashed reference line.
Fig. 3.15 also shows the unreliable (red) and reliable (green) regions.

Method (note Sec. 3.2.2): profile <chi2/ndf> vs edge, 5-point moving window over the bins with entries, scanning from
large to small edge; cut at the centre of the first window with variance > 0.05 (the last bin above threshold).
The scan starts at 20 cm (R1, R2) or 30 cm (R3).

Writes (<out_dir>):
  fig3_15_moving_variance_LD2_R1_S1.png/.pdf  profile (blue) and moving variance (purple), LD2 IB, R1 S1, 0-25 cm
  fig3_16_sectors_<sample>.png/.pdf            profile per sector, R1-R3, 0-25 cm   (IB targets and LD2 OB)
  fig3_17_momentum_LD2.png/.pdf                profile per 1 GeV momentum bin, R1-R3, LD2 IB
  fig3_18_theta_LD2.png/.pdf                   profile per 5 deg theta bin, R1-R3, LD2 IB
  fig3_19_targets_S1.png/.pdf                  profile for sector 1, R1-R3, LD2 / CxC / CuSn IB
  dc_occupancy_LD2.png/.pdf                   entries in (p, edge) and (theta, edge), R1-R3, LD2 IB (where the 3D histograms have data)
  table3_dc_cuts.csv                           preliminary cuts per sample, region, sector
Prints: entries per sector and region for each sample; the preliminary Table 3.
"""
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

if len(sys.argv) != 4:
    sys.exit("usage: python3 dc_figs.py <root_dir> <ob_ld2_file> <out_dir>")
ROOT, OBFILE, OUT = Path(sys.argv[1]), Path(sys.argv[2]), Path(sys.argv[3])
OUT.mkdir(parents=True, exist_ok=True)

TARGETS = {"LD2": ("D (LD2), run 18335", "black"),
           "CxC": ("C (CxC), run 18344", "#1f77b4"),
           "CuSn": ("CuSn, run 18384", "#9467bd")}
SECTOR_COLORS = plt.get_cmap("tab10").colors[:6]
NOTE_CUT = {1: 1.88, 2: 2.08, 3: 8.62}
WIN, VAR_MAX, MIN_N = 5, 0.05, 1
EMAX = {1: 20.0, 2: 20.0, 3: 30.0}
XMAX = 25.0

samples = {t: uproot.open(ROOT / f"ib_fig_{t}.root") for t in TARGETS}
samples["LD2_OB"] = uproot.open(OBFILE)


def profile_2d(v, ex, ey):
    """Mean of y, its error (RMS/sqrt(N)) and N in each x bin."""
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
    """Moving variance over the scanned bins, and the cut (first window above threshold, from large edge)."""
    ok = (n >= MIN_N) & (x < EMAX[r]) & np.isfinite(m)
    xs, ms = x[ok], m[ok]
    var = np.full(len(xs), np.nan)
    cut = np.nan
    for i in range(len(xs) - WIN, -1, -1):
        var[i + WIN // 2] = np.var(ms[i:i + WIN])
        if np.isnan(cut) and var[i + WIN // 2] > VAR_MAX:
            cut = xs[i + WIN // 2]
    return cut, xs, var


def draw(ax, x, m, e, n, color, label):
    ok = (n > 0) & (x <= XMAX)
    ax.errorbar(x[ok], m[ok], yerr=e[ok], fmt="o", ms=2, lw=0.8, color=color, label=label)


def style(ax, title, ylabel=r"$\langle\chi^2/ndf\rangle$"):
    ax.set_xlim(0, XMAX)
    ax.set_xlabel("edge [cm]")
    ax.set_ylabel(ylabel)
    ax.set_title(title, fontsize=9)
    ax.grid(alpha=0.3)
    ax.legend(fontsize=7, frameon=False)


def cut_of(prof, r):
    x, m, e, n = prof
    return moving_variance(x, m, n, r)[0]


def cut_line(ax, c, color):
    if np.isfinite(c):
        ax.axvline(c, color=color, ls="--", lw=1)


def cut_box(ax, lines):
    ax.text(0.02, 0.97, "Cut values\n" + "\n".join(lines), transform=ax.transAxes, va="top", ha="left",
            fontsize=7, family="monospace", bbox=dict(boxstyle="round", fc="white", ec="gray", alpha=0.85))


def fmt(c):
    return f"{c:5.2f} cm" if np.isfinite(c) else "  none"


def note_line(ax, r):
    ax.axvline(NOTE_CUT[r], color="k", ls="--", lw=1, label=f"note OB cut {NOTE_CUT[r]} cm")


def save(fig, name):
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=140)
    plt.close(fig)
    print(f"wrote {OUT / name}.png/.pdf")


# ---- entries per sector and region
print("=== entries (trigger electrons) per sector, ElectronTracks/edge_vs_chi2ndf")
for name, f in samples.items():
    for r in (1, 2, 3):
        ns = [int(sector_profile(f, r, s)[3].sum()) for s in range(1, 7)]
        print(f"    {name:8s} R{r}: " + "  ".join(f"S{s} {n:8d}" for s, n in enumerate(ns, 1)))

# ---- preliminary cuts (printed and saved only)
cuts = {(name, r, s): moving_variance(*[a for i, a in enumerate(sector_profile(f, r, s)) if i != 2], r)[0]
        for name, f in samples.items() for r in (1, 2, 3) for s in range(1, 7)}
rows = ["sample,region," + ",".join(f"S{s}" for s in range(1, 7)) + ",average,note_OB"]
print("\n=== PRELIMINARY Table 3 (statistics too low, see figures): sector average [cm]")
for name in samples:
    av = []
    for r in (1, 2, 3):
        cs = [cuts[(name, r, s)] for s in range(1, 7)]
        av.append(np.nanmean(cs))
        rows.append(f"{name},{r}," + ",".join(f"{c:.2f}" for c in cs) + f",{np.nanmean(cs):.2f},{NOTE_CUT[r]}")
    print(f"    {name:8s} " + "   ".join(f"R{r} {a:6.2f}" for r, a in zip((1, 2, 3), av)))
(OUT / "table3_dc_cuts.csv").write_text("\n".join(rows) + "\n")
print(f"wrote {OUT / 'table3_dc_cuts.csv'}\n")

# ---- Fig. 3.15: method illustration, LD2 IB, R1 S1
x, m, e, n = sector_profile(samples["LD2"], 1, 1)
cut, xs, var = moving_variance(x, m, n, 1)
fig, axs = plt.subplots(2, 1, figsize=(9, 7.5), sharex=True)
draw(axs[0], x, m, e, n, "tab:blue", "Data")
for ax in axs:
    ax.axvspan(0, cut, color="red", alpha=0.08)
    ax.axvspan(cut, XMAX, color="green", alpha=0.08)
axs[0].axvline(cut, color="green", ls="--", label=f"cut from moving variance: {cut:.2f} cm")
axs[0].axvline(NOTE_CUT[1], color="gray", ls=":", label=f"note (OB): {NOTE_CUT[1]} cm")
axs[0].plot([], [], "s", color="red", alpha=0.2, label="unreliable (cut)")
axs[0].plot([], [], "s", color="green", alpha=0.2, label="reliable (keep)")
style(axs[0], "D (LD2) IB run 18335, region 1, sector 1: profile")
axs[0].set_ylim(0, 7)
axs[0].set_xlabel("")
okd = (n > 0) & (x <= XMAX)
xd, md = x[okd], m[okd]
vd = np.full(len(xd), np.nan)
for i in range(len(xd) - WIN + 1):
    vd[i + WIN // 2] = np.var(md[i:i + WIN])
axs[1].plot(xd, vd, "o-", ms=3, color="tab:purple", label="moving variance (window = 5 bins), all bins")
axs[1].axhline(VAR_MAX, color="k", ls=":", label="threshold 0.05")
axs[1].axvline(cut, color="green", ls="--")
axs[1].axvline(NOTE_CUT[1], color="gray", ls=":", label=f"note (OB): {NOTE_CUT[1]} cm")
style(axs[1], "moving variance over every bin with entries, 0-25 cm", ylabel="variance")
axs[1].set_ylim(-0.02, 1.1 * np.nanmax(vd))
save(fig, "fig3_15_moving_variance_LD2_R1_S1")

# ---- Fig. 3.16: sectors per region, per sample
for name, f in samples.items():
    lab = TARGETS[name][0] + ", IB" if name in TARGETS else "D (LD2), OB (" + OBFILE.name + ")"
    fig, axs = plt.subplots(1, 3, figsize=(18, 4.8))
    for r, ax in zip((1, 2, 3), axs):
        lines = []
        for s in range(1, 7):
            prof = sector_profile(f, r, s)
            c = cut_of(prof, r)
            draw(ax, *prof, SECTOR_COLORS[s - 1], f"Sector {s}")
            cut_line(ax, c, SECTOR_COLORS[s - 1])
            lines.append(f"S{s}: {fmt(c)}")
        note_line(ax, r)
        cut_box(ax, lines)
        style(ax, f"{lab}: region {r}")
    save(fig, f"fig3_16_sectors_{name}")


# ---- Figs. 3.17 and 3.18: every momentum and theta bin of the histogram, LD2 IB
def h3_profiles(f, kind, r):
    h = f[f"DC/chi2ndf_vs_edge_vs_{kind}_r{r}"]
    v, e0, e1, e2 = h.values(), h.axis(0).edges(), h.axis(1).edges(), h.axis(2).edges()
    return [((e0[k], e0[k + 1]), *profile_2d(v[k], e1, e2)) for k in range(len(e0) - 1)]


for kind, unit, sym, fname in (("p", "GeV", "p", "fig3_17_momentum_LD2"),
                               ("theta", "deg", r"\theta", "fig3_18_theta_LD2")):
    fig, axs = plt.subplots(1, 3, figsize=(18, 4.8))
    for r, ax in zip((1, 2, 3), axs):
        prof = h3_profiles(samples["LD2"], kind, r)
        cmap = plt.get_cmap("viridis")
        lines = []
        for k, ((lo, hi), x, m, e, n) in enumerate(prof):
            col = cmap(k / max(len(prof) - 1, 1))
            c = cut_of((x, m, e, n), r)
            draw(ax, x, m, e, n, col, rf"${sym} \in [{lo:g}, {hi:g})$ {unit}")
            cut_line(ax, c, col)
            lines.append(f"[{lo:g},{hi:g}): {fmt(c)}")
        note_line(ax, r)
        cut_box(ax, lines)
        style(ax, f"D (LD2) IB run 18335, region {r}", ylabel=r"$\langle\chi^2/ndf\rangle$ (full track)")
    save(fig, fname)

# ---- Fig. 3.19: targets, sector 1
fig, axs = plt.subplots(1, 3, figsize=(18, 4.8))
for r, ax in zip((1, 2, 3), axs):
    lines = []
    for t, (lab, col) in TARGETS.items():
        prof = sector_profile(samples[t], r, 1)
        c = cut_of(prof, r)
        draw(ax, *prof, col, lab)
        cut_line(ax, c, col)
        lines.append(f"{t:4s}: {fmt(c)}")
    note_line(ax, r)
    cut_box(ax, lines)
    style(ax, f"Sector 1, IB: region {r}")
    if r == 3:
        ax.set_ylim(0, 4)
save(fig, "fig3_19_targets_S1")

# ---- occupancy: where the 3D histograms (Figs. 3.17, 3.18) have entries, LD2 IB
from matplotlib.colors import LogNorm
fig, axs = plt.subplots(2, 3, figsize=(18, 9))
for row, (kind, ylab) in enumerate((("p", "p [GeV]"), ("theta", r"$\theta$ [deg]"))):
    for r in (1, 2, 3):
        h = samples["LD2"][f"DC/chi2ndf_vs_edge_vs_{kind}_r{r}"]
        v = h.values().sum(axis=2)
        e0, e1 = h.axis(0).edges(), h.axis(1).edges()
        ax = axs[row, r - 1]
        mesh = ax.pcolormesh(e1, e0, np.where(v > 0, v, np.nan), norm=LogNorm(), cmap="viridis")
        fig.colorbar(mesh, ax=ax, label="entries")
        ax.axvline(NOTE_CUT[r], color="red", ls="--", lw=1, label=f"note OB cut {NOTE_CUT[r]} cm")
        ax.set_xlim(0, XMAX)
        ax.set_xlabel("edge [cm]")
        ax.set_ylabel(ylab)
        ax.set_title(f"D (LD2) IB run 18335, region {r}: entries", fontsize=9)
        ax.legend(fontsize=7, loc="upper right")
save(fig, "dc_occupancy_LD2")
