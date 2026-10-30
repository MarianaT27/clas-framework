import sys
import numpy as np
import uproot
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

f_ib, f_ob = uproot.open(sys.argv[1]), uproot.open(sys.argv[2])
NOTE_CUT = {1: 1.88, 2: 2.08, 3: 8.62}   # common note, Table 3 (OB)
WIN, VAR_MAX, MIN_ENTRIES = 5, 0.05, 50


def find(f, name):
    for k in f.keys(recursive=True, cycle=False):
        if k.endswith(name):
            return f[k]
    sys.exit(f"{name} not found in {f.file_path}")


def dc_profile(f, r):
    h = find(f, f"chi2ndf_vs_edge_vs_p_r{r}")
    v = h.values().sum(axis=0)                 # (edge, chi2)
    edge, chi2 = h.axis(1).centers(), h.axis(2).centers()
    n = v.sum(axis=1)
    mean = np.where(n > MIN_ENTRIES, (v * chi2).sum(axis=1) / np.maximum(n, 1), np.nan)
    return edge, mean, n


def moving_variance_cut(edge, mean):
    ok = np.where(np.isfinite(mean))[0]
    for i in ok:
        w = mean[i:i + WIN]
        if len(w) == WIN and np.all(np.isfinite(w)) and np.var(w) < VAR_MAX:
            return edge[i]
    return np.nan


fig, ax = plt.subplots(2, 3, figsize=(14, 8))
print("=== DC edge threshold from moving variance (all sectors combined)")
print("region   IB [cm]   OB [cm]   note OB [cm]")
for k, r in enumerate((1, 2, 3)):
    cuts = {}
    for lab, f, c in (("IB", f_ib, "tab:red"), ("OB", f_ob, "tab:blue")):
        e, m, n = dc_profile(f, r)
        cuts[lab] = moving_variance_cut(e, m)
        ax[0, k].plot(e, m, color=c, label=lab)
        pass
    ax[0, k].axvline(NOTE_CUT[r], color="k", ls="--", label=f"OB cut (note) {NOTE_CUT[r]} cm")
    ax[0, k].set_xlim(0, 20)
    ax[0, k].set_ylim(0, 5)
    ax[0, k].set_xlabel("edge [cm]")
    ax[0, k].set_ylabel(r"$\langle\chi^2/ndf\rangle$")
    ax[0, k].set_title(f"DC region {r}")
    ax[0, k].legend(frameon=False, fontsize=8)
    print(f"  R{r}     {cuts['IB']:6.2f}    {cuts['OB']:6.2f}    {NOTE_CUT[r]:6.2f}")

for k, (name, coord) in enumerate((("lu_vs_lv_pcal", "lv"), ("lu_vs_lw_pcal", "lw"))):
    for lab, f, c in (("IB", f_ib, "tab:red"), ("OB", f_ob, "tab:blue")):
        h = find(f, name)
        v = h.values().sum(axis=1)               # x axis is lv (or lw), 5 cm bins
        x = h.axis(0).centers()
        ax[1, k].step(x, v / v.sum(), where="mid", color=c, label=lab)
    ax[1, k].axvline(9.0, color="k", ls="--", label="9 cm")
    ax[1, k].set_xlim(0, 150)
    ax[1, k].set_xlabel(f"PCAL {coord} [cm]")
    ax[1, k].set_ylabel("fraction")
    ax[1, k].legend(frameon=False, fontsize=8)

ax[1, 2].axis("off")
fig.suptitle("Electron fiducial variables, IB vs OB (fiducial cuts off)", fontsize=11)
fig.tight_layout()
for ext in ("pdf", "png"):
    fig.savefig(f"fid_ib_ob.{ext}", dpi=130)
print("\nwrote fid_ib_ob.pdf/.png")
