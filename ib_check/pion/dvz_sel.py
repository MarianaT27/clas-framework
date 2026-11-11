"""
dvz_sel.py : pion Delta Vz after the pion Vz cut, common note Sec. 5.3 (Eq. 13, Table 9, Fig. 5.13), offline from
the pion n-tuples (needs the v1.4 n-tuple with e_vz).

Usage (from the clas-framework folder):
    python3 ib_check/pion/dvz_sel.py /volatile/clas12/mtenorio/pi0/ib_check/pion [data/Vz/vz_corrector_params.txt]

Reads   <dir>/root/pion_ana_<T>_<POL>.root, trees pion_plus_analysis / pion_minus_analysis
          (pi_p, pi_theta, pi_phi, pi_vz, pi_sector, e_vz)
        the 12-block Vz params file (pion blocks <variant>_pim / _pip).
Vz cut  LD2: raw pion vz in (-15, 5) cm. CxC / CuSn: pion Vz correction (numpy copy of VzCorrector::apply, as in
        vz_pion_sel.py) and the Table 8 windows mu +- 3 sigma_eff; pions with pi_sector outside 1-6 are dropped.
        Rows: LD2, CxC (either foil window), Cu (upstream window), Sn (downstream window).
        The trigger electron already passed its own Vz cut in pion_analysis.
Delta Vz = e_vz - pi_vz (raw vertices, Eq. 13), 0.1 cm bins in [-20, 20] cm.
Method  as in the note: Gaussian fit over the top 50% of the peak, window mu +- 3 sigma. Selections: all, p >= 1 GeV.
Writes  <dir>/dvz/dvz_sel_<POL>.png/.pdf : rows LD2, CxC, Cu, Sn; columns pi-, pi+; "all" (target color, red fit,
          dashed window) and "p >= 1" (gray); dotted = Table 9 window (OB).
Prints  mu, sigma, window and fraction of pions passing the Vz cut, per polarity, target, charge and selection,
        next to Table 9 (OB only in the note).
"""
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import uproot
from scipy.optimize import curve_fit

DIR = Path(sys.argv[1])
PARAMS = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("data/Vz/vz_corrector_params.txt")
OUT = DIR / "dvz"
OUT.mkdir(parents=True, exist_ok=True)
POLS = ["IB", "OB"]
ROWS = [("LD2", "LD2"), ("CxC", "CxC"), ("Cu", "CuSn"), ("Sn", "CuSn")]
COLOR = {"LD2": "black", "CxC": "#1f77b4", "CuSn": "#9467bd"}
SPECIES = [("pim", "pion_minus_analysis", r"$\pi^-$", "PiMinus"), ("pip", "pion_plus_analysis", r"$\pi^+$", "PiPlus")]
SECTOR_PHI_CENTER = np.array([0.0, 60.0, 120.0, 180.0, -120.0, -60.0])
N_PHI, PHI_HALF, PHI_BINW = 6, 30.0, 10.0
EDGES = np.arange(-20.0, 20.0 + 1e-9, 0.1)
XC = 0.5 * (EDGES[1:] + EDGES[:-1])
TABLE8 = {  # (mu_up, sigma_eff_up, mu_down, sigma_eff_down) [cm], common note Table 8
    ("CuSn", "OB", "pim"): (-7.777, 0.449, -2.823, 0.382), ("CuSn", "OB", "pip"): (-7.782, 0.403, -2.821, 0.393),
    ("CuSn", "IB", "pim"): (-7.830, 0.463, -2.921, 0.425), ("CuSn", "IB", "pip"): (-7.840, 0.426, -2.951, 0.401),
    ("CxC", "OB", "pim"): (-7.815, 0.412, -2.805, 0.389), ("CxC", "OB", "pip"): (-7.783, 0.364, -2.802, 0.409),
    ("CxC", "IB", "pim"): (-7.744, 0.463, -2.782, 0.437), ("CxC", "IB", "pip"): (-7.799, 0.421, -2.849, 0.410),
}
TABLE9 = {  # common note Table 9, OB: (mu, sigma, window lo, window hi) [cm]
    ("LD2", "PiMinus"): (0.18, 1.34, -3.85, 4.21), ("LD2", "PiPlus"): (-0.85, 1.05, -3.99, 2.29),
    ("CxC", "PiMinus"): (0.19, 1.33, -3.81, 4.19), ("CxC", "PiPlus"): (-0.84, 1.05, -3.98, 2.30),
    ("Cu", "PiMinus"): (0.20, 1.34, -3.82, 4.21), ("Cu", "PiPlus"): (-0.85, 1.03, -3.95, 2.26),
    ("Sn", "PiMinus"): (0.17, 1.26, -3.60, 3.94), ("Sn", "PiPlus"): (-0.81, 1.04, -3.93, 2.30),
}


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


def gauss(x, a, m, s):
    return a * np.exp(-0.5 * ((x - m) / s) ** 2)


def fit_top_half(y, xc):
    ys = np.convolve(y, np.ones(3) / 3, mode="same")
    i0 = int(np.argmax(ys))
    half = 0.5 * ys[i0]
    lo = i0
    while lo > 0 and ys[lo - 1] >= half:
        lo -= 1
    hi = i0
    while hi < len(y) - 1 and ys[hi + 1] >= half:
        hi += 1
    x, yy = xc[lo:hi + 1], y[lo:hi + 1]
    s0 = max((xc[hi] - xc[lo]) / 2.355, 0.1)
    p, cov = curve_fit(gauss, x, yy, p0=[ys[i0], xc[i0], s0], sigma=np.sqrt(np.maximum(yy, 1)))
    p[2] = abs(p[2])
    chi2 = np.sum(((yy - gauss(x, *p)) / np.sqrt(np.maximum(yy, 1))) ** 2) / max(len(x) - 3, 1)
    return p, np.sqrt(np.diag(cov)), (xc[lo], xc[hi]), chi2



def find(f, name):
    for k in f.keys(recursive=True, cycle=False):
        if k.split(";")[0].endswith(name):
            return f[k]
    return None


blocks = load_params(PARAMS)
cache = {}  # (pol, file target, tag) -> {row name: {"all": hist, "p>=1": hist}}, n_total
rows = []
for pol in POLS:
    for t in ["LD2", "CxC", "CuSn"]:
        f = uproot.open(DIR / "root" / f"pion_ana_{t}_{pol}.root")
        names = {"LD2": ["LD2"], "CxC": ["CxC"], "CuSn": ["Cu", "Sn"]}[t]
        var = ("cusn" if t == "CuSn" else "cxc") + ("_outbending" if pol == "OB" else "_inbending")
        for tag, tree_name, _, _ in SPECIES:
            tree = find(f, tree_name)
            if "e_vz" not in tree.keys():
                sys.exit(f"{tree_name} in pion_ana_{t}_{pol}.root has no e_vz: rerun pion_analysis with v1.4")
            h = {n: {"all": np.zeros(len(XC)), "p>=1": np.zeros(len(XC))} for n in names}
            n_tot = 0
            for a in tree.iterate(["pi_p", "pi_theta", "pi_phi", "pi_vz", "pi_sector", "e_vz"],
                                  step_size="200 MB", library="np"):
                n_tot += len(a["pi_vz"])
                dvz = a["e_vz"] - a["pi_vz"]
                if t == "LD2":
                    sel = {"LD2": (a["pi_vz"] > -15.0) & (a["pi_vz"] < 5.0)}
                else:
                    ref = TABLE8[(t, pol, tag)]
                    sec = a["pi_sector"].astype(int)
                    ok = (sec >= 1) & (sec <= 6)
                    vzc = np.full(len(dvz), np.nan)
                    vzc[ok] = apply(blocks[f"{var}_{tag}"], ref, a["pi_vz"][ok], a["pi_p"][ok], a["pi_theta"][ok],
                                    a["pi_phi"][ok], sec[ok])
                    up = np.abs(vzc - ref[0]) < 3 * ref[1]
                    dn = np.abs(vzc - ref[2]) < 3 * ref[3]
                    sel = {"CxC": up | dn} if t == "CxC" else {"Cu": up, "Sn": dn}
                for n, m in sel.items():
                    h[n]["all"] += np.histogram(dvz[m], EDGES)[0]
                    h[n]["p>=1"] += np.histogram(dvz[m & (a["pi_p"] >= 1.0)], EDGES)[0]
            cache[(pol, tag)] = cache.get((pol, tag), {})
            for n in names:
                cache[(pol, tag)][n] = (h[n], n_tot)
            print(f"done {pol} {t} {tag} ({n_tot} pions)")

for pol in POLS:
    fig, axs = plt.subplots(len(ROWS), 2, figsize=(10, 3.0 * len(ROWS)), squeeze=False)
    for i, (name, t) in enumerate(ROWS):
        for j, (tag, _, lab, sp) in enumerate(SPECIES):
            ax = axs[i, j]
            hh, n_tot = cache[(pol, tag)][name]
            note = TABLE9.get((name, sp)) if pol == "OB" else None
            for s, col in (("all", COLOR[t]), ("p>=1", "gray")):
                y = hh[s]
                ax.stairs(y, EDGES, color=col, lw=0.9, label=s)
                try:
                    p, e, (flo, fhi), chi2 = fit_top_half(y, XC)
                except (RuntimeError, ValueError):
                    rows.append((pol, name, sp, s, 100 * y.sum() / max(n_tot, 1), np.nan, np.nan, np.nan, note))
                    continue
                m, sg = p[1], p[2]
                rows.append((pol, name, sp, s, 100 * y.sum() / max(n_tot, 1), m, sg, chi2, note))
                if s == "all":
                    xx = np.linspace(flo, fhi, 200)
                    ax.plot(xx, gauss(xx, *p), color="red", lw=1.5,
                            label=rf"$\mu$={m:.3f}, $\sigma$={sg:.3f} cm")
                    for v in (m - 3 * sg, m + 3 * sg):
                        ax.axvline(v, color="gray", ls="--", lw=0.9)
            if note:
                for v in note[2:]:
                    ax.axvline(v, color="green", ls=":", lw=1.1)
            ax.set_xlim(-20, 20)
            ax.set_title(rf"{pol}, {name}, {lab} (after Vz cut)", fontsize=10)
            ax.legend(fontsize=7, loc="upper right")
            ax.tick_params(labelsize=8)
            if i == len(ROWS) - 1:
                ax.set_xlabel(r"$\Delta V_z = V_z(e^-) - V_z(\pi)$ [cm]")
    leg = "dashed = this work $\\mu\\pm3\\sigma$ (all)" + (", dotted = Table 9" if pol == "OB" else "")
    fig.suptitle(f"{pol}: $\\Delta V_z$ after the pion Vz cut, top-50% Gaussian fit (red); {leg}", fontsize=11)
    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(OUT / f"dvz_sel_{pol}.{ext}", dpi=140, bbox_inches="tight")
    plt.close(fig)
    print("wrote", OUT / f"dvz_sel_{pol}.png")

print("\nDelta Vz = Vz(e-) - Vz(pi) after the pion Vz cut vs common note Table 9 (OB only), cm")
print(f"{'pol':3} {'target':6} {'species':8} {'sel':5} {'pass':>6} | {'mu':>6} {'sigma':>6} {'window':>16} {'chi2/ndf':>8} | "
      f"{'mu':>6} {'sigma':>6} {'window (Table 9)':>16}")
for pol, name, sp, s, kept, m, sg, chi2, note in rows:
    nt = f"{note[0]:6.2f} {note[1]:6.2f} [{note[2]:5.2f}, {note[3]:5.2f}]" if note else f"{'-':>6} {'-':>6} {'-':>16}"
    print(f"{pol:3} {name:6} {sp:8} {s:5} {kept:5.1f}% | {m:6.2f} {sg:6.2f} [{m - 3 * sg:6.2f}, {m + 3 * sg:5.2f}] "
          f"{chi2:8.1f} | {nt}")
