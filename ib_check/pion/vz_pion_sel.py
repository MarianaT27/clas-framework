"""
vz_pion_sel.py : effect of |chi2pid| < 10 and p >= 1 GeV on the corrected pion Vz widths, from the pion n-tuples
already in pion_ana_<T>_<POL>.root (no rerun).

Usage (from the clas-framework folder):
    python3 ib_check/pion/vz_pion_sel.py /volatile/clas12/mtenorio/pi0/ib_check/pion [data/Vz/vz_corrector_params.txt]

Reads   <dir>/root/pion_ana_<T>_<POL>.root, trees pion_plus_analysis / pion_minus_analysis
          (pi_p, pi_theta, pi_phi, pi_vz, pi_chi2pid, pi_sector; all pions after the pion_analysis pre-selection)
        the 12-block Vz params file (pion blocks <variant>_pim / _pip).
Applies the pion Vz correction in numpy, line by line as VzCorrector::apply, with the Table 8 reference (mu, sigma_eff)
        that VzCorrectorService sets with set_cut. Pions with pi_sector outside 1-6 are skipped.
Selections: all | |chi2pid| < 10 | p >= 1 GeV | both.
Prints  Eq. 8 fit (two Voigts + pedestal) of the corrected Vz per target, polarity, charge and selection:
        mu, sigma_eff per foil, fraction of pions kept by the selection, next to Table 8.
Writes  <dir>/vz/vz_pion_sel.png/.pdf : one panel per (target, polarity, charge), the 4 selections overlaid
        (normalized), gray lines at the Table 8 centroids.
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import uproot
from scipy.optimize import curve_fit
from scipy.special import voigt_profile

DIR = Path(sys.argv[1])
PARAMS = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("data/Vz/vz_corrector_params.txt")
OUT = DIR / "vz"
OUT.mkdir(parents=True, exist_ok=True)
TARGETS, POLS = ["CxC", "CuSn"], ["IB", "OB"]
SPECIES = [("pim", "pion_minus_analysis", r"$\pi^-$"), ("pip", "pion_plus_analysis", r"$\pi^+$")]
SELS = [("all", "black"), ("|chi2pid|<10", "#1f77b4"), ("p>=1", "#ff7f0e"), ("both", "#2ca02c")]
TABLE8 = {  # (mu_up, sigma_eff_up, mu_down, sigma_eff_down) [cm], common note Table 8
    ("CuSn", "OB", "pim"): (-7.777, 0.449, -2.823, 0.382), ("CuSn", "OB", "pip"): (-7.782, 0.403, -2.821, 0.393),
    ("CuSn", "IB", "pim"): (-7.830, 0.463, -2.921, 0.425), ("CuSn", "IB", "pip"): (-7.840, 0.426, -2.951, 0.401),
    ("CxC", "OB", "pim"): (-7.815, 0.412, -2.805, 0.389), ("CxC", "OB", "pip"): (-7.783, 0.364, -2.802, 0.409),
    ("CxC", "IB", "pim"): (-7.744, 0.463, -2.782, 0.437), ("CxC", "IB", "pip"): (-7.799, 0.421, -2.849, 0.410),
}
EDGES = np.arange(-12.0, 1.0 + 1e-9, 0.05)
CENTERS = 0.5 * (EDGES[1:] + EDGES[:-1])
SECTOR_PHI_CENTER = np.array([0.0, 60.0, 120.0, 180.0, -120.0, -60.0])
N_PHI, PHI_HALF, PHI_BINW = 6, 30.0, 10.0


def load_params(path):
    lines = [l.split() for l in open(path) if l.strip() and not l.lstrip().startswith("#")]
    n_var = int(lines[0][0])
    i, blocks = 1, {}
    for _ in range(n_var):
        h = lines[i]
        key, n_p = h[0], int(h[7])
        n_cells = 6 * n_p * N_PHI
        rows = np.array(lines[i + 1:i + 1 + n_cells], dtype=float)
        idx = ((rows[:, 0].astype(int) - 1) * n_p * N_PHI + rows[:, 1].astype(int) * N_PHI + rows[:, 2].astype(int))
        cells = np.empty((n_cells, 18))
        cells[idx] = rows[:, 3:]
        blocks[key] = dict(p_lo=float(h[5]), p_binw=float(h[6]), n_p=n_p, cells=cells)
        i += 1 + n_cells
    return blocks


def apply(b, ref, vz, p, th, phi, sec):
    """VzCorrector::apply, vectorized. ref = (mu_up, s_up, mu_dn, s_dn); lo = downstream, hi = upstream."""
    s = np.clip(sec, 1, 6)
    ip = np.clip(((p - b["p_lo"]) / b["p_binw"]).astype(int), 0, b["n_p"] - 1)
    r = np.mod(phi - SECTOR_PHI_CENTER[s - 1] + 540.0, 360.0) - 180.0
    r = np.clip(r, -PHI_HALF, PHI_HALF - 1e-9)
    iphi = np.clip(((r + PHI_HALF) / PHI_BINW).astype(int), 0, N_PHI - 1)
    c = b["cells"][(s - 1) * b["n_p"] * N_PHI + ip * N_PHI + iphi]
    tmin = np.where(np.isfinite(c[:, 16]), c[:, 16], 6.0)
    tmax = np.where(np.isfinite(c[:, 17]), c[:, 17], 30.0)
    t = np.clip(th, tmin, tmax)

    def poly(k):
        return c[:, k] + t * (c[:, k + 1] + t * (c[:, k + 2] + t * c[:, k + 3]))

    mu_lo, mu_hi = poly(0), poly(4)
    sig_lo, sig_hi = np.maximum(poly(8), 0.01), np.maximum(poly(12), 0.01)
    mu0_hi, s0_hi, mu0_lo, s0_lo = ref
    z_lo, z_hi = (vz - mu_lo) / sig_lo, (vz - mu_hi) / sig_hi
    v_lo, v_hi = mu0_lo + s0_lo * z_lo, mu0_hi + s0_hi * z_hi
    w_lo = 1.0 / (1.0 + np.exp(-(0.5 * (z_hi**2 - z_lo**2) + np.log(sig_hi / sig_lo))))
    return v_hi + w_lo * (v_lo - v_hi)


def model(x, a1, m1, s1, g1, a2, m2, s2, g2, c):  # Eq. 8
    return a1 * voigt_profile(x - m1, s1, g1) + a2 * voigt_profile(x - m2, s2, g2) + c


def sigma_eff(s, g):  # Eq. 9
    fl, fg = 2 * g, 2 * np.sqrt(2 * np.log(2)) * s
    return (0.5346 * fl + np.sqrt(0.2166 * fl**2 + fg**2)) / (2 * np.sqrt(2 * np.log(2)))


def fit(y, start):
    sel = (CENTERS >= -11.0) & (CENTERS <= 0.0) & (y > 0)
    x, yy = CENTERS[sel], y[sel]
    mu1, s1, mu2, s2 = start
    n = yy.sum() * 0.05
    p0 = [0.5 * n, mu1, 0.7 * s1, 0.3 * s1, 0.5 * n, mu2, 0.7 * s2, 0.3 * s2, max(np.median(yy) * 0.1, 1.0)]
    lo = [0, mu1 - 1, 0.01, 0.0, 0, mu2 - 1, 0.01, 0.0, 0]
    hi = [np.inf, mu1 + 1, 2, 2, np.inf, mu2 + 1, 2, 2, np.inf]
    try:
        p, _ = curve_fit(model, x, yy, p0=p0, bounds=(lo, hi), sigma=np.sqrt(yy), maxfev=40000)
    except (RuntimeError, ValueError):
        return (np.nan,) * 4
    return p[1], sigma_eff(p[2], p[3]), p[5], sigma_eff(p[6], p[7])


def find(f, name):
    for k in f.keys(recursive=True, cycle=False):
        if k.split(";")[0].endswith(name):
            return f[k]
    return None


blocks = load_params(PARAMS)
rows = []
fig, axs = plt.subplots(len(TARGETS) * len(POLS), 2, figsize=(12, 3.0 * len(TARGETS) * len(POLS)), squeeze=False)
row = 0
for t in TARGETS:
    for pol in POLS:
        fn = DIR / "root" / f"pion_ana_{t}_{pol}.root"
        f = uproot.open(fn)
        var = ("cusn" if t == "CuSn" else "cxc") + ("_outbending" if pol == "OB" else "_inbending")
        for j, (tag, tree_name, lab) in enumerate(SPECIES):
            key = f"{var}_{tag}"
            if key not in blocks:
                sys.exit(f"no block {key} in {PARAMS}")
            ref = TABLE8[(t, pol, tag)]
            tree = find(f, tree_name)
            hists = {s: np.zeros(len(CENTERS)) for s, _ in SELS}
            n_tot = 0
            for a in tree.iterate(["pi_p", "pi_theta", "pi_phi", "pi_vz", "pi_chi2pid", "pi_sector"],
                                  step_size="200 MB", library="np"):
                ok = (a["pi_sector"] >= 1) & (a["pi_sector"] <= 6)
                p, th, phi, vz = a["pi_p"][ok], a["pi_theta"][ok], a["pi_phi"][ok], a["pi_vz"][ok]
                chi, sec = a["pi_chi2pid"][ok], a["pi_sector"][ok].astype(int)
                vzc = apply(blocks[key], ref, vz, p, th, phi, sec)
                n_tot += len(vzc)
                masks = {"all": np.ones(len(vzc), bool), "|chi2pid|<10": np.abs(chi) < 10, "p>=1": p >= 1.0}
                masks["both"] = masks["|chi2pid|<10"] & masks["p>=1"]
                for s, _ in SELS:
                    hists[s] += np.histogram(vzc[masks[s]], EDGES)[0]
            ax = axs[row, j]
            for s, col in SELS:
                y = hists[s]
                ax.stairs(y / (y.sum() * 0.05) if y.sum() else y, EDGES, color=col, lw=1, label=s)
                rows.append((t, pol, tag, s, 100 * y.sum() / max(n_tot, 1), fit(y, ref), ref))
            for m in (ref[0], ref[2]):
                ax.axvline(m, color="gray", ls="--", lw=0.8)
            ax.set_xlim(-11.5, 0)
            ax.set_title(f"{t} {pol}, {lab}", fontsize=10)
            ax.legend(fontsize=7, loc="upper left")
            print(f"done {t} {pol} {tag} ({n_tot} pions)")
        row += 1
for ax in axs[-1]:
    ax.set_xlabel(r"corrected $v_z$ [cm]")
fig.suptitle("Pion corrected $v_z$ (pion correction) per selection; gray = Table 8 centroids", fontsize=11)
fig.tight_layout()
for ext in ("png", "pdf"):
    fig.savefig(OUT / f"vz_pion_sel.{ext}", dpi=130, bbox_inches="tight")
print("wrote", OUT / "vz_pion_sel.png")

print("\nEq. 8 fit of the pion-corrected vz per selection vs Table 8, cm (kept = % of the pions in the n-tuple)")
print(f"{'target':6} {'pol':3} {'sp':3} {'selection':13} {'kept':>6} | {'mu_up':>7} {'seff_up':>7} {'note':>6} | "
      f"{'mu_dn':>7} {'seff_dn':>7} {'note':>6}")
for t, pol, tag, s, kept, o, n in rows:
    print(f"{t:6} {pol:3} {tag:3} {s:13} {kept:5.1f}% | {o[0]:7.3f} {o[1]:7.3f} {n[1]:6.3f} | "
          f"{o[2]:7.3f} {o[3]:7.3f} {n[3]:6.3f}")
