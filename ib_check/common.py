"""
common.py : settings shared by the ib_check figure scripts (polarity, runs, targets, code SF parameters).

Every script takes the polarity as its second argument (IB or OB) and reads <root_dir>/<ib|ob>_fig_<T>.root.
An optional fourth argument replaces the run of one or more targets, e.g. "LD2=18305,CxC=18401": those targets are
then read from <root_dir>/<ib|ob>_fig_<T>_<run>.root and labelled with that run; the other targets keep the default.
"""
import sys
from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

RUNS = {"IB": {"LD2": 18335, "CxC": 18344, "CuSn": 18384},
        "OB": {"LD2": 18543, "CxC": 18465, "CuSn": 18591}}
NAMES = {"LD2": "D (LD2)", "CxC": "C (CxC)", "CuSn": "CuSn"}
COLORS = {"LD2": "black", "CxC": "#1f77b4", "CuSn": "#9467bd"}
SECTOR_COLORS = plt.get_cmap("tab10").colors[:6]

# SamplingFractionService.hpp: m_data["OB"] = note Table 13 "Average", m_data["IB"] = note Table 14 "Average".
# mu(p) = a + b p + c p^2 + d p^3, same for sigma(p). The code cuts at mu +- 3.5 sigma for both polarities.
N_SIGMA_SF = 3.5
CODE_SF = {
 "OB": {"mu": [[2.300896E-01, 4.862074E-03, -6.441645E-04, 1.247456E-05],
               [2.286452E-01, 4.985634E-03, -3.668113E-04, -1.110160E-05],
               [2.258053E-01, 9.073840E-03, -1.240745E-03, 3.579321E-05],
               [2.281016E-01, 6.073295E-03, -5.916479E-04, 2.569914E-06],
               [2.302059E-01, 3.020237E-03, -4.307581E-05, -2.216133E-05],
               [2.262583E-01, 6.849767E-03, -8.027577E-04, 1.288497E-05]],
        "sig": [[2.226749E-02, -3.186742E-03, 3.087387E-04, -1.247691E-05],
                [2.150934E-02, -2.549354E-03, 1.855833E-04, -4.394923E-06],
                [2.399538E-02, -4.208668E-03, 4.503779E-04, -1.783519E-05],
                [2.217527E-02, -2.788589E-03, 2.266746E-04, -7.717066E-06],
                [2.277357E-02, -3.941980E-03, 4.455085E-04, -1.912118E-05],
                [2.253050E-02, -3.334553E-03, 3.374127E-04, -1.413483E-05]]},
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


RUN_OVERRIDE = {}   # (polarity, target) -> run, from the optional fourth argument


def args(extra_usage=""):
    """<root_dir> <IB|OB> <out_dir> [T=run,...]  ->  (root_dir, polarity, out_dir); out_dir is created."""
    usage = f"usage: python3 {Path(sys.argv[0]).name} <root_dir> <IB|OB> <out_dir> [T=run,...]{extra_usage}"
    if len(sys.argv) not in (4, 5) or sys.argv[2] not in ("IB", "OB"):
        sys.exit(usage)
    root, pol, out = Path(sys.argv[1]), sys.argv[2], Path(sys.argv[3])
    if len(sys.argv) == 5:
        for item in sys.argv[4].split(","):
            t, _, r = item.partition("=")
            if t not in NAMES or not r.isdigit():
                sys.exit(f"bad run override '{item}' (expected e.g. LD2=18305)\n{usage}")
            RUN_OVERRIDE[(pol, t)] = int(r)
    out.mkdir(parents=True, exist_ok=True)
    return root, pol, out


def run_of(pol, t):
    return RUN_OVERRIDE.get((pol, t), RUNS[pol][t])


def fname(root, pol, t):
    if (pol, t) in RUN_OVERRIDE:
        return root / f"{pol.lower()}_fig_{t}_{RUN_OVERRIDE[(pol, t)]}.root"
    return root / f"{pol.lower()}_fig_{t}.root"


def label(pol, t):
    return f"{NAMES[t]}, run {run_of(pol, t)}, {pol}"


def cubic(p, a, b, c, d):
    return a + b * p + c * p**2 + d * p**3


def code_band(pol, sector, p):
    """Code mu(p) and sigma(p) for this polarity and sector."""
    return cubic(p, *CODE_SF[pol]["mu"][sector - 1]), cubic(p, *CODE_SF[pol]["sig"][sector - 1])


# Slice fit of the sampling fraction (sf_fits.py, sf_slices.py): Gaussian in a FIXED symmetric window
# seed +- SF_HALF_WINDOW, seeded at the code mean of the polarity (not at the highest bin), re-centred on the
# fitted mean once; the window width never grows with the fitted sigma, so a low-SF population next to the
# electron peak stays outside the fit.
SF_HALF_WINDOW = 0.02
SF_FIT_ITER = 2


def gauss(x, n, mu, s):
    return n * np.exp(-0.5 * ((x - mu) / s) ** 2)


def sf_peak_fit(sc, y, seed):
    """Fit y(sc) around seed -> (popt, pcov, window mask). Raises RuntimeError if the window has < 5 filled bins."""
    from scipy.optimize import curve_fit
    mu = seed
    for _ in range(SF_FIT_ITER):
        w = (sc > mu - SF_HALF_WINDOW) & (sc < mu + SF_HALF_WINDOW) & (y > 0)
        if w.sum() < 5:
            raise RuntimeError("too few filled bins in the fit window")
        popt, pcov = curve_fit(gauss, sc[w], y[w], p0=[y[w].max(), mu, 0.5 * SF_HALF_WINDOW], sigma=np.sqrt(y[w]))
        mu = popt[1]
    return popt, pcov, w


def save(fig, out, name, dpi=140):
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(out / f"{name}.{ext}", dpi=dpi)
    plt.close(fig)
    print(f"wrote {out / name}.png/.pdf")


def h1(f, name):
    h = f[name]
    return h.values(), h.axis().edges()


def h2(f, name):
    h = f[name]
    return h.values(), h.axis(0).edges(), h.axis(1).edges()


def masked(v):
    """Empty bins as NaN, so they stay white on a log colour scale."""
    return np.where(v > 0, v, np.nan).T
