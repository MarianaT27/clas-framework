"""
photon_figs.py : figures and loss table of the photon identification check (photon_check/run_photon.sh).

Usage:
    python3 photon_check/photon_figs.py <photon_check_dir> <out_dir>
    e.g. python3 photon_check/photon_figs.py /volatile/clas12/mtenorio/pi0/photon_check /volatile/clas12/mtenorio/pi0/photon_check/figs

Reads:  <photon_check_dir>/runs.txt                        the run list run_photon.sh used ("<T> <OB|IB> <run>")
        <photon_check_dir>/root/photon_<T>_<POL>_<run>.root  PhotonAnalysis/... histograms
Writes, for each polarity in runs.txt (<pol> = ob or ib), all targets of that polarity in each figure:
        <out_dir>/<pol>_prefilter.pdf      GBT pre-filter: E_gamma, E_PCAL, theta (all pid 22, and after the pre-filter)
        <out_dir>/<pol>_gbt_score.pdf      GBT score of the photons passing the pre-filter
        <out_dir>/<pol>_pcal_fiducial.pdf  PCAL lv and lw per sector, before the fiducial cut (one row per target)
        <out_dir>/<pol>_beta.pdf           beta, before the beta cut
        <out_dir>/<pol>_e_gamma_angle.pdf  angle between the selected electron and the photon, before the cut
        <out_dir>/loss_table.csv           cutflow of every target and polarity (also printed)
        (a .png next to every .pdf)
Every distribution is the photons that REACH that cut, i.e. after all the previous ones (PhotonAnalysisAlg.hpp).
All distributions are raw photon counts (not normalized); the dashed lines are the cuts.
"""
import csv
import sys
from pathlib import Path

import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

if len(sys.argv) != 3:
    sys.exit(__doc__)
BASE, OUT = Path(sys.argv[1]), Path(sys.argv[2])
OUT.mkdir(parents=True, exist_ok=True)

NAMES = {"LD2": "D (LD2)", "CxC": "C (CxC)", "CuSn": "CuSn"}
COLORS = {"LD2": "black", "CxC": "#1f77b4", "CuSn": "#9467bd"}   # same as ib_check/common.py
SECTOR_COLORS = plt.get_cmap("tab10").colors[:6]
ORDER = ["LD2", "CxC", "CuSn"]

# The cuts drawn on the figures: the PhotonAnalysisAlg defaults (photon_check/PROVENANCE.md).
E_MIN, THETA_MIN, THETA_MAX, GBT_CUT, PCAL_MIN, BETA_MIN, BETA_MAX, EG_MIN = 0.2, 5.0, 35.0, 0.78, 14.0, 0.9, 1.1, 8.0

STEPS = ["pid 22", "E >= 0.2 GeV", "E_PCAL > 0", "5 <= theta <= 35 deg", "GBT score > 0.78",
         "PCAL lv, lw > 14 cm", "0.9 < beta < 1.1", "theta(e,gamma) > 8 deg"]

# runs.txt -> {pol: [(target, run), ...]}
RUNS = {}
for line in (BASE / "runs.txt").read_text().splitlines():
    parts = line.split()
    if len(parts) < 3 or parts[0].startswith("#"):
        continue
    t, pol, run = parts[0], parts[1], int(parts[2])
    if (BASE / "root" / f"photon_{t}_{pol}_{run}.root").exists():
        RUNS.setdefault(pol, []).append((t, run))
    else:
        print(f"no output for {t} {pol} run {run}, skipped")
for pol in RUNS:
    RUNS[pol].sort(key=lambda tr: ORDER.index(tr[0]) if tr[0] in ORDER else 99)


def open_run(t, pol, run):
    return uproot.open(BASE / "root" / f"photon_{t}_{pol}_{run}.root")


def h1(f, name):
    h = f[f"PhotonAnalysis/{name}"]
    return h.values(), h.axis().edges()


def draw(ax, v, e, **kw):
    ax.stairs(v, e, **kw)


def cut_lines(ax, *xs):
    for x in xs:
        ax.axvline(x, color="red", ls="--", lw=1)


def save(fig, name):
    fig.tight_layout()
    for ext in ("pdf", "png"):
        fig.savefig(OUT / f"{name}.{ext}", dpi=140)
    plt.close(fig)
    print(f"wrote {OUT / name}.pdf")


def label(t, pol, run):
    return f"{NAMES.get(t, t)}, run {run}"


rows = []
for pol, runs in RUNS.items():
    p = pol.lower()
    files = {(t, run): open_run(t, pol, run) for t, run in runs}

    # 2. GBT pre-filter
    fig, axes = plt.subplots(1, 3, figsize=(15, 4.5))
    for (t, run), f in files.items():
        for ax, var in zip(axes, ("E", "Epcal", "theta")):
            vb, eb = h1(f, f"{var}_before")
            va, _ = h1(f, f"{var}_after")
            ax.stairs(vb, eb, color=COLORS.get(t), lw=1.2, label=f"{label(t, pol, run)}, pid 22")
            ax.stairs(va, eb, color=COLORS.get(t), lw=1.2, ls=":", label=f"{label(t, pol, run)}, after pre-filter")
    axes[0].set_xlabel(r"$E_\gamma$ [GeV]"); cut_lines(axes[0], E_MIN)
    axes[1].set_xlabel(r"$E_{PCAL}$ [GeV]")
    axes[2].set_xlabel(r"$\theta$ [deg]"); cut_lines(axes[2], THETA_MIN, THETA_MAX)
    for ax in axes:
        ax.set_ylabel("photons")
    axes[0].legend(fontsize=7)
    fig.suptitle(f"{pol}: GBT pre-filter")
    save(fig, f"{p}_prefilter")

    # 3. GBT score
    fig, ax = plt.subplots(figsize=(6.5, 4.5))
    for (t, run), f in files.items():
        v, e = h1(f, "score")
        draw(ax, v, e, color=COLORS.get(t), lw=1.2, label=label(t, pol, run))
    cut_lines(ax, GBT_CUT)
    ax.set_xlabel("GBT score"); ax.set_ylabel("photons")
    ax.legend(fontsize=8); ax.set_title(f"{pol}: GBT score after the pre-filter")
    save(fig, f"{p}_gbt_score")

    # 4. PCAL fiducial, per sector
    fig, axes = plt.subplots(len(files), 2, figsize=(12, 3.6 * len(files)), squeeze=False)
    for row, ((t, run), f) in enumerate(files.items()):
        for col, coord in enumerate(("lv", "lw")):
            ax = axes[row, col]
            for s in range(1, 7):
                v, e = h1(f, f"PCAL/{coord}_before_S{s}")
                draw(ax, v, e, color=SECTOR_COLORS[s - 1], lw=1.0, label=f"sector {s}")
            cut_lines(ax, PCAL_MIN)
            ax.set_xlabel(f"PCAL $l_{coord[1]}$ [cm]"); ax.set_ylabel("photons")
            ax.set_title(f"{label(t, pol, run)}, {pol}", fontsize=10)
    axes[0, 0].legend(fontsize=7)
    save(fig, f"{p}_pcal_fiducial")

    # 5. beta, 6. angle to the electron
    for var, xlabel, cuts, name in (("beta_before", r"$\beta$", (BETA_MIN, BETA_MAX), "beta"),
                                    ("e_gamma_angle_before", r"$\theta_{e\gamma}$ [deg]", (EG_MIN,), "e_gamma_angle")):
        fig, ax = plt.subplots(figsize=(6.5, 4.5))
        for (t, run), f in files.items():
            v, e = h1(f, var)
            draw(ax, v, e, color=COLORS.get(t), lw=1.2, label=label(t, pol, run))
        cut_lines(ax, *cuts)
        ax.set_xlabel(xlabel); ax.set_ylabel("photons")
        ax.legend(fontsize=8); ax.set_title(f"{pol}: {xlabel}, before the cut")
        save(fig, f"{p}_{name}")

    # loss table
    for (t, run), f in files.items():
        counts, _ = h1(f, "cutflow")
        n_dis = h1(f, "n_dis_events")[0].sum()
        for i, step in enumerate(STEPS):
            n = counts[i]
            prev = counts[i - 1] if i > 0 else n
            rows.append({"target": t, "polarity": pol, "run": run, "DIS events": int(n_dis), "step": step,
                         "photons": int(n),
                         "kept vs previous [%]": f"{100 * n / prev:.2f}" if prev > 0 else "",
                         "kept vs pid 22 [%]": f"{100 * n / counts[0]:.2f}" if counts[0] > 0 else ""})

with open(OUT / "loss_table.csv", "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()) if rows else ["target"])
    w.writeheader()
    w.writerows(rows)
print(f"wrote {OUT / 'loss_table.csv'}")
for r in rows:
    print(f"{r['target']:5s} {r['polarity']} {r['run']}  {r['step']:24s} {r['photons']:>12d}  "
          f"{r['kept vs previous [%]']:>7s}%  {r['kept vs pid 22 [%]']:>7s}%")
