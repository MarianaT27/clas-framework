import uproot
import numpy as np
import matplotlib.pyplot as plt
import mplhep as hep
import scienceplots
import matplotlib as mpl

# ── Style ──────────────────────────────────────────────────────────────────
hep.style.use("CMSTex")
plt.style.use(["science"])
mpl.rc("text", usetex=True)
mpl.rc("text.latex", preamble=r"\usepackage{amsmath} \usepackage{lmodern}")
plt.rcParams.update({
    "figure.dpi": 150, "savefig.dpi": 300, "font.size": 12,
    "xtick.direction": "in", "ytick.direction": "in",
    "xtick.top": False, "ytick.right": False,
})

PION_SIM_VZ_HIST: dict[str, tuple[str, str]] = {
    "pim": ("MC/PiMinus/vz_Sn", "MC/PiMinus/vz_Cu"),
    "pip": ("MC/PiPlus/vz_Sn",  "MC/PiPlus/vz_Cu"),
}
ELECTRON_SIM_VZ_HIST: tuple[str, str] = ("MC/Electron/vz_Sn", "MC/Electron/vz_Cu")
SIM_PATH: str = "/volatile/clas12/ouillon/CuSn_contamination/CuSn_contamination_CuSn_Outbending.root"

SPECIES_LABEL: dict[str, str] = {
    "Electron": r"$e^-$",
    "PiPlus":   r"$\pi^+$",
    "PiMinus":  r"$\pi^-$",
}


# Mirror of CuSnContamination::fmt_p_edge: 0.5 -> "0p5", 10.0 -> "10p0".
def _fmt_edge(v: float) -> str:
    return f"{v:.1f}".replace(".", "p")


# tab10 (10 hex codes) + black + saddle-brown so adjacent lines stay readable
# for up to 12 bins (viridis washes out at the bright end).
_BIN_PALETTE: list[str] = [
    "#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd",
    "#8c564b", "#e377c2", "#7f7f7f", "#bcbd22", "#17becf",
    "#000000", "#8B4513",
]


def _bin_colors(n: int) -> list[str]:
    if n <= len(_BIN_PALETTE):
        return _BIN_PALETTE[:n]
    raise ValueError(f"_bin_colors only supports up to {len(_BIN_PALETTE)} bins (asked for {n})")


def main():
    with uproot.open(SIM_PATH) as f:
        h_pim_sn, h_pim_cu = f[PION_SIM_VZ_HIST["pim"][0]], f[PION_SIM_VZ_HIST["pim"][1]]
        h_pip_sn, h_pip_cu = f[PION_SIM_VZ_HIST["pip"][0]], f[PION_SIM_VZ_HIST["pip"][1]]
        h_pim_sn_nocuts, h_pim_cu_nocuts = f["MC/PiMinus_NoCuts/vz_Sn"], f["MC/PiMinus_NoCuts/vz_Cu"]
        h_pip_sn_nocuts, h_pip_cu_nocuts = f["MC/PiPlus_NoCuts/vz_Sn"], f["MC/PiPlus_NoCuts/vz_Cu"]

        fig, axs = plt.subplots(1, 2, figsize=(10, 5))
        hep.histplot(h_pim_sn.to_numpy(), label="Sn", ax=axs[0], density=True)
        hep.histplot(h_pim_cu.to_numpy(), label="Cu", ax=axs[0], density=True)
        hep.histplot(h_pim_sn_nocuts.to_numpy(), label="Sn (no cuts, MC pid = -211)", ax=axs[0], color="red", density=True)
        hep.histplot(h_pim_cu_nocuts.to_numpy(), label="Cu (no cuts, MC pid = -211)", ax=axs[0], color="orange", density=True)
        hep.histplot(h_pip_sn.to_numpy(), label="Sn", ax=axs[1], density=True)
        hep.histplot(h_pip_cu.to_numpy(), label="Cu", ax=axs[1], density=True)
        hep.histplot(h_pip_sn_nocuts.to_numpy(), label="Sn (no cuts, MC pid = 211)", ax=axs[1], color="red", density=True)
        hep.histplot(h_pip_cu_nocuts.to_numpy(), label="Cu (no cuts, MC pid = 211)", ax=axs[1], color="orange", density=True)
        axs[0].set_title(r"$\pi^-$")
        axs[1].set_title(r"$\pi^+$")
        for ax in axs:
            ax.set_xlabel(r"$v_z$ [cm]")
            ax.set_ylabel("Counts")
            ax.legend()
        fig.suptitle("Simulated $v_z$ distribution for pions")
        fig.savefig("simulated_vz_pions.pdf", bbox_inches="tight")


def plot_vz_theta_slices(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    theta_min: float = 5.0,
    theta_max: float = 35.0,
    theta_step: float = 5.0,
    density: bool = True,
    species_suffix: str = "",
    out_path: str = "vz_Cu_vs_theta_slices.pdf",
    title: str = r"Simulated $v_z$ in $\theta$ slices",
):
    """Plot Vz distributions sliced in theta bins for each species.

    Assumes the 2D histogram /MC/<species><species_suffix>/vz_CuSn_vs_theta is
    stored with theta on one axis and vz on the other (auto-detected).
    """
    edges = np.arange(theta_min, theta_max + 1e-9, theta_step)
    slices = list(zip(edges[:-1], edges[1:]))
    cmap = mpl.cm.tab10
    colors = [cmap(i % 10) for i in range(len(slices))]

    fig, axs = plt.subplots(1, len(species), figsize=(5 * len(species), 5), squeeze=False)
    axs = axs[0]

    with uproot.open(file_path) as f:
        for ax, sp in zip(axs, species):
            h2 = f[f"MC/{sp}{species_suffix}/vz_CuSn_vs_theta"]
            values, xedges, yedges = h2.to_numpy()

            # Auto-detect which axis is theta vs vz by their value ranges.
            # vz is typically in [-15, 5] cm; theta is positive (deg or rad).
            x_is_vz = xedges.min() < -1.0
            y_is_vz = yedges.min() < -1.0
            if x_is_vz and not y_is_vz:
                theta_edges, vz_edges = yedges, xedges
                # values shape (n_vz, n_theta) → transpose to (n_theta, n_vz)
                vals = values.T
            elif y_is_vz and not x_is_vz:
                theta_edges, vz_edges = xedges, yedges
                vals = values
            else:
                raise RuntimeError(
                    f"Cannot identify vz axis for {sp}: "
                    f"xedges=[{xedges.min():.3g},{xedges.max():.3g}] "
                    f"yedges=[{yedges.min():.3g},{yedges.max():.3g}]"
                )

            # Detect radians vs degrees from max value (>2π implies degrees).
            in_radians = theta_edges.max() < 6.5
            print(
                f"[{sp}] theta on {'x' if not x_is_vz else 'y'} axis, "
                f"range [{theta_edges.min():.3g}, {theta_edges.max():.3g}] "
                f"({'rad' if in_radians else 'deg'}); "
                f"vz range [{vz_edges.min():.3g}, {vz_edges.max():.3g}]; "
                f"total entries = {vals.sum():.0f}"
            )

            for (t_lo, t_hi), color in zip(slices, colors):
                lo = np.deg2rad(t_lo) if in_radians else t_lo
                hi = np.deg2rad(t_hi) if in_radians else t_hi
                i_lo = np.searchsorted(theta_edges, lo, side="left")
                i_hi = np.searchsorted(theta_edges, hi, side="left")
                if i_hi <= i_lo:
                    continue
                proj = vals[i_lo:i_hi, :].sum(axis=0)
                if density and proj.sum() > 0:
                    widths = np.diff(vz_edges)
                    proj = proj / (proj.sum() * widths)
                hep.histplot(
                    (proj, vz_edges),
                    ax=ax,
                    label=rf"${t_lo:.0f} \leq \theta < {t_hi:.0f}^\circ$",
                    color=color,
                    histtype="step",
                )

            ax.set_title(SPECIES_LABEL.get(sp, sp))
            ax.set_xlabel(r"$v_z$ [cm]")
            ax.set_ylabel("PDF" if density else "Counts")
            ax.legend(fontsize=8, ncol=1, loc="best")

    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_quality_sliced_vz(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    targets: tuple[str, ...] = ("Cu", "Sn"),
    density: bool = True,
    out_path: str = "vz_quality_slices.pdf",
):
    """Overlay quality-sliced Data cut sample vs the MC 'no cuts MC pid' reference.

    Per (species, target): plot reference (MC NoCuts) + Q-Hi + Q-Mid + Q-Lo.
    If Q-Hi matches the reference, broadening lives in Q-Mid + Q-Lo.
    """
    quality_styles = {
        "QHi": ("Q $\\geq$ 0.92", "tab:blue", "-"),
        "QLo": ("Q $<$ 0.50",    "tab:red",  "-"),
    }
    ref_style = ("MC no cuts (truth pid)", "black", "--")

    fig, axs = plt.subplots(
        len(species), len(targets),
        figsize=(5 * len(targets), 4 * len(species)),
        squeeze=False,
    )

    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            for j, tgt in enumerate(targets):
                ax = axs[i][j]

                hep.histplot(
                    f[f"MC/{sp}_NoCuts/vz_{tgt}"].to_numpy(),
                    ax=ax, label=ref_style[0],
                    color=ref_style[1], linestyle=ref_style[2],
                    histtype="step", density=density,
                )
                for q, (lbl, color, ls) in quality_styles.items():
                    h = f[f"MC/{sp}_{q}/vz_{tgt}"]
                    hep.histplot(
                        h.to_numpy(),
                        ax=ax, label=lbl,
                        color=color, linestyle=ls,
                        histtype="step", density=density,
                    )

                ax.set_title(f"{SPECIES_LABEL.get(sp, sp)} — {tgt}")
                ax.set_xlabel(r"$v_z$ [cm]")
                ax.set_ylabel("PDF" if density else "Counts")
                ax.legend(fontsize=8, loc="best")

    fig.suptitle(r"$v_z$ by reconstruction quality")
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_vz_per_p_bin(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    p_edges: tuple[float, ...] = tuple(np.arange(0.0, 6.5, 0.5)),
    density: bool = True,
    base_dir: str = "Data",
    species_suffix: str = "",
    out_path: str = "vz_per_p_bin.pdf",
    title: str = r"$v_z$ by momentum bin (cut sample)",
):
    """Overlay reconstructed Vz per 0.5 GeV momentum bin.

    Reads histograms from /<base_dir>/<species><species_suffix>/vz_p{lo}_{hi}
    where {lo} / {hi} use the "0p5" decimal-replacement encoding from
    `CuSnContamination::fmt_p_edge`.
    """
    bins = list(zip(p_edges[:-1], p_edges[1:]))
    colors = _bin_colors(len(bins))

    fig, axs = plt.subplots(1, len(species), figsize=(5 * len(species), 5), squeeze=False)
    axs = axs[0]

    with uproot.open(file_path) as f:
        for ax, sp in zip(axs, species):
            for (p_lo, p_hi), color in zip(bins, colors):
                key = f"{base_dir}/{sp}{species_suffix}/vz_p{_fmt_edge(p_lo)}_{_fmt_edge(p_hi)}"
                h = f[key]
                hep.histplot(
                    h.to_numpy(),
                    ax=ax, label=rf"${p_lo:.1f} \leq p < {p_hi:.1f}$ GeV",
                    color=color, histtype="step", density=density,
                )

            ax.set_title(SPECIES_LABEL.get(sp, sp))
            ax.set_xlabel(r"$v_z$ [cm]")
            ax.set_ylabel("PDF" if density else "Counts")
            ax.legend(fontsize=7, ncol=1, loc="best")

    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def _project_2d_slices(
    h2,
    slice_edges: list[tuple[float, float]],
    residual_axis: str = "auto",
    slice_unit_hint: str | None = None,
    density: bool = True,
) -> list[tuple[np.ndarray, np.ndarray]]:
    """Project a 2-D (binning x residual) uproot histogram onto residual per slice.

    `residual_axis` selects which axis holds the projected quantity:
      - "auto" (default): infer by which axis has min < -1.0 (works for vz).
      - "x": x axis holds the residual; y axis is the binning.
      - "y": y axis holds the residual; x axis is the binning.

    Use "y" for the Residual/* 2-D histograms — Δp/p has |range| < 1
    so the auto-detect can't tell which axis is the residual.

    `slice_unit_hint="theta_deg"` converts deg slice edges to rad when the
    histogram's binning axis turns out to be in radians.
    """
    values, xedges, yedges = h2.to_numpy()
    if residual_axis == "auto":
        x_is_res = xedges.min() < -1.0
        y_is_res = yedges.min() < -1.0
        if x_is_res and not y_is_res:
            residual_axis = "x"
        elif y_is_res and not x_is_res:
            residual_axis = "y"
        else:
            raise RuntimeError(
                f"Cannot identify residual axis: "
                f"xedges=[{xedges.min():.3g},{xedges.max():.3g}] "
                f"yedges=[{yedges.min():.3g},{yedges.max():.3g}]"
            )

    if residual_axis == "x":
        bin_edges, res_edges = yedges, xedges
        vals = values.T
    elif residual_axis == "y":
        bin_edges, res_edges = xedges, yedges
        vals = values
    else:
        raise ValueError(f"residual_axis must be 'auto', 'x', or 'y' (got {residual_axis!r})")

    if slice_unit_hint == "theta_deg" and bin_edges.max() < 6.5:
        slice_edges = [(np.deg2rad(lo), np.deg2rad(hi)) for lo, hi in slice_edges]

    out: list[tuple[np.ndarray, np.ndarray]] = []
    widths = np.diff(res_edges)
    for lo, hi in slice_edges:
        i_lo = np.searchsorted(bin_edges, lo, side="left")
        i_hi = np.searchsorted(bin_edges, hi, side="left")
        if i_hi <= i_lo:
            out.append((np.zeros(len(res_edges) - 1), res_edges))
            continue
        proj = vals[i_lo:i_hi, :].sum(axis=0)
        if density and proj.sum() > 0:
            proj = proj / (proj.sum() * widths)
        out.append((proj, res_edges))
    return out


THREE_WAY_CATEGORIES: tuple[tuple[str, str], ...] = (
    ("A", "Cut sample"),
    ("B", "Rec pions (MC pid)"),
    ("C", "Truth pions (any rec pid)"),
    ("D", "MC truth"),
)


def plot_vz_three_way_integrated(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    density: bool = True,
    out_path: str = "vz_three_way_integrated.pdf",
):
    """Overlay integrated vz for the four pion populations per species."""
    styles = {
        "A": ("Cut sample",                "tab:blue",   "-"),
        "B": ("Rec pions (MC pid)",        "tab:orange", "-"),
        "C": ("Truth pions (any rec pid)", "tab:green",  "-"),
        "D": ("MC truth",                  "black",      "--"),
    }

    fig, axs = plt.subplots(1, len(species), figsize=(5 * len(species), 5), squeeze=False)
    axs = axs[0]

    with uproot.open(file_path) as f:
        for ax, sp in zip(axs, species):
            sources = {
                "A": f[f"Data/{sp}/vz_CuSn"],
                "B": f[f"Data/{sp}_NoCuts/vz_CuSn"],
                "C": f[f"Data/{sp}_Truth/vz_CuSn"],
                "D": f[f"MC/{sp}/vz"],
            }
            for tag, h in sources.items():
                lbl, color, ls = styles[tag]
                hep.histplot(
                    h.to_numpy(), ax=ax, label=lbl,
                    color=color, linestyle=ls, histtype="step", density=density,
                )
            ax.set_title(SPECIES_LABEL.get(sp, sp))
            ax.set_xlabel(r"$v_z$ [cm]")
            ax.set_ylabel("PDF" if density else "Counts")
            ax.legend(fontsize=9, loc="best")

    fig.suptitle(r"Integrated $v_z$: four-way comparison")
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_vz_three_way_p(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    p_edges: tuple[float, ...] = tuple(np.arange(0.0, 6.5, 0.5)),
    density: bool = True,
    out_path: str = "vz_three_way_p.pdf",
):
    """Per-species grid (rows) x category (cols); overlay vz per p bin in each cell."""
    bins = list(zip(p_edges[:-1], p_edges[1:]))
    colors = _bin_colors(len(bins))

    n_cols = len(THREE_WAY_CATEGORIES)
    fig, axs = plt.subplots(
        len(species), n_cols,
        figsize=(4.5 * n_cols, 4 * len(species)),
        squeeze=False,
    )

    rec_base_dirs = {
        "A": lambda sp: f"Data/{sp}",
        "B": lambda sp: f"Data/{sp}_NoCuts",
        "C": lambda sp: f"Data/{sp}_Truth",
    }

    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            # MC truth is a 2D (p, vz) histogram — project once per species.
            cat_d_slices = _project_2d_slices(
                f[f"MC/{sp}/vz_vs_p"], list(bins), density=density,
            )
            for j, (tag, lbl) in enumerate(THREE_WAY_CATEGORIES):
                ax = axs[i][j]
                for k, ((p_lo, p_hi), color) in enumerate(zip(bins, colors)):
                    legend = rf"${p_lo:.1f} \leq p < {p_hi:.1f}$ GeV"
                    if tag == "D":
                        proj, vz_edges = cat_d_slices[k]
                        hep.histplot(
                            (proj, vz_edges), ax=ax, label=legend,
                            color=color, histtype="step",
                        )
                    else:
                        h = f[f"{rec_base_dirs[tag](sp)}/vz_p{_fmt_edge(p_lo)}_{_fmt_edge(p_hi)}"]
                        hep.histplot(
                            h.to_numpy(), ax=ax, label=legend,
                            color=color, histtype="step", density=density,
                        )

                if i == 0:
                    ax.set_title(lbl)
                ax.set_xlabel(r"$v_z$ [cm]")
                if j == 0:
                    ax.set_ylabel(rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts"))
                else:
                    ax.set_ylabel("PDF" if density else "Counts")
                ax.legend(fontsize=7, ncol=1, loc="best")

    fig.suptitle(r"$v_z$ by momentum bin: four-way comparison")
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_vz_three_way_theta(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    theta_min: float = 5.0,
    theta_max: float = 35.0,
    theta_step: float = 5.0,
    density: bool = True,
    out_path: str = "vz_three_way_theta.pdf",
):
    """Per-species (rows) x category (cols); overlay vz per theta bin in each cell."""
    edges = np.arange(theta_min, theta_max + 1e-9, theta_step)
    slices = list(zip(edges[:-1], edges[1:]))
    cmap = mpl.cm.tab10
    colors = [cmap(i % 10) for i in range(len(slices))]

    n_cols = len(THREE_WAY_CATEGORIES)
    fig, axs = plt.subplots(
        len(species), n_cols,
        figsize=(4.5 * n_cols, 4 * len(species)),
        squeeze=False,
    )

    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            cell_paths = {
                "A": f[f"MC/{sp}/vz_CuSn_vs_theta"],
                "B": f[f"MC/{sp}_NoCuts/vz_CuSn_vs_theta"],
                "C": f[f"MC/{sp}_Truth/vz_CuSn_vs_theta"],
                "D": f[f"MC/{sp}/vz_vs_theta"],
            }
            cell_projections = {
                tag: _project_2d_slices(h2, list(slices),
                                        slice_unit_hint="theta_deg",
                                        density=density)
                for tag, h2 in cell_paths.items()
            }
            for j, (tag, lbl) in enumerate(THREE_WAY_CATEGORIES):
                ax = axs[i][j]
                for (t_lo, t_hi), color, (proj, vz_edges) in zip(slices, colors, cell_projections[tag]):
                    hep.histplot(
                        (proj, vz_edges), ax=ax,
                        label=rf"${t_lo:.0f} \leq \theta < {t_hi:.0f}^\circ$",
                        color=color, histtype="step",
                    )
                if i == 0:
                    ax.set_title(lbl)
                ax.set_xlabel(r"$v_z$ [cm]")
                if j == 0:
                    ax.set_ylabel(rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts"))
                else:
                    ax.set_ylabel("PDF" if density else "Counts")
                ax.legend(fontsize=7, ncol=1, loc="best")

    fig.suptitle(r"$v_z$ by $\theta$ bin: four-way comparison")
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


# Cumulative pion-cut-step columns: (species suffix used by CusnContaminationAlg,
# short column title). The last entry maps to the unsuffixed `Data/PiPlus/...`
# i.e. the final SelectedPi* sample (also adds the Δt cut).
PION_CUT_STEP_COLUMNS: tuple[tuple[str, str], ...] = (
    ("_noPionCuts",     r"rec pid pion"),
    ("_plusDCFiducial", r"+ DC fid"),
    ("_plusChi2PID",    r"+ $\chi^2_{\rm PID}$"),
    ("_plusMomentum",   r"+ momentum"),
    ("_plusVertexZ",    r"+ $v_z$"),
    ("",                r"+ $\Delta t$ (cut)"),
)


def plot_vz_cut_steps_p(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    p_edges: tuple[float, ...] = tuple(np.arange(0.0, 6.5, 0.5)),
    density: bool = True,
    out_path: str = "vz_cut_steps_p.pdf",
    title: str = r"$v_z$ by momentum bin after each pion cut (forward pions only)",
):
    """2 rows (species) x 6 cols (cut steps); overlay vz per p bin in each cell.

    Reads from `Data/{species}{suffix}/vz_p{lo}_{hi}` where the suffix walks
    through the cumulative cut chain (PionSelectionAlg) and the final
    unsuffixed entry is the SelectedPi* sample.
    """
    bins = list(zip(p_edges[:-1], p_edges[1:]))
    colors = _bin_colors(len(bins))
    n_cols = len(PION_CUT_STEP_COLUMNS)
    fig, axs = plt.subplots(
        len(species), n_cols,
        figsize=(3.5 * n_cols, 4 * len(species)),
        squeeze=False,
    )
    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            for j, (suffix, label) in enumerate(PION_CUT_STEP_COLUMNS):
                ax = axs[i][j]
                for (p_lo, p_hi), color in zip(bins, colors):
                    legend = rf"${p_lo:.1f} \leq p < {p_hi:.1f}$"
                    h = f[f"Data/{sp}{suffix}/vz_p{_fmt_edge(p_lo)}_{_fmt_edge(p_hi)}"]
                    hep.histplot(
                        h.to_numpy(), ax=ax, label=legend,
                        color=color, histtype="step", density=density,
                    )
                if i == 0:
                    ax.set_title(label)
                ax.set_xlabel(r"$v_z$ [cm]")
                ax.set_ylabel(
                    rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts")
                    if j == 0 else ("PDF" if density else "Counts")
                )
                ax.legend(fontsize=6, ncol=1, loc="best")
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_vz_cut_steps_per_p(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    p_edges: tuple[float, ...] = (0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0),
    density: bool = False,
    out_path: str = "vz_cut_steps_per_p_overlay.pdf",
    title: str | None = None,
):
    """Per (species, p bin) panel: overlay every cut step as a separate line.

    Designed for low-p diagnostic. Default `density=False` so the
    cumulative yield reduction is visible — flip to True to compare
    shape evolution after re-normalization.
    """
    bins = list(zip(p_edges[:-1], p_edges[1:]))
    cmap = mpl.cm.tab10
    cut_colors = [cmap(k % 10) for k in range(len(PION_CUT_STEP_COLUMNS))]

    fig, axs = plt.subplots(
        len(species), len(bins),
        figsize=(3.5 * len(bins), 4 * len(species)),
        squeeze=False,
    )
    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            for j, (p_lo, p_hi) in enumerate(bins):
                ax = axs[i][j]
                for (suffix, step_label), color in zip(PION_CUT_STEP_COLUMNS, cut_colors):
                    h = f[f"Data/{sp}{suffix}/vz_p{_fmt_edge(p_lo)}_{_fmt_edge(p_hi)}"]
                    hep.histplot(
                        h.to_numpy(), ax=ax, label=step_label,
                        color=color, histtype="step", density=density,
                    )
                if i == 0:
                    ax.set_title(rf"${p_lo:.1f} \leq p < {p_hi:.1f}$ GeV")
                ax.set_xlabel(r"$v_z$ [cm]")
                ax.set_ylabel(
                    rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts")
                    if j == 0 else ("PDF" if density else "Counts")
                )
                ax.legend(fontsize=6, ncol=1, loc="best")
    fig.suptitle(title or (
        r"$v_z$ — pion cuts applied successively per $p$ bin "
        + ("(density=True, shape only)" if density else "(counts)")
    ))
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


RESIDUAL_VARS: tuple[tuple[str, str], ...] = (
    ("dp_over_p", r"$\Delta p / p$"),
    ("dphi",      r"$\Delta \phi$ [deg]"),
    ("dtheta",    r"$\Delta \theta$ [deg]"),
    ("dvz",       r"$\Delta v_z$ [cm]"),
)


def plot_residuals_integrated(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    species_suffix: str = "",
    density: bool = True,
    out_path: str = "residuals_integrated.pdf",
    title: str = r"Reconstruction residuals (integrated, cut sample)",
):
    """2x4 grid (rows = species, cols = vars): integrated residuals.

    `species_suffix` selects the Residual sub-directory:
      - "" (default): cut sample (rec pid pion AND passes the pion cuts)
      - "_Truth": every rec track whose MC match is a true pion
    """
    fig, axs = plt.subplots(
        len(species), len(RESIDUAL_VARS),
        figsize=(4 * len(RESIDUAL_VARS), 3.5 * len(species)),
        squeeze=False,
    )
    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            for j, (var, xlabel) in enumerate(RESIDUAL_VARS):
                ax = axs[i][j]
                h = f[f"Residual/{sp}{species_suffix}/{var}"]
                hep.histplot(
                    h.to_numpy(), ax=ax, histtype="step",
                    color="tab:blue", density=density,
                )
                if i == 0:
                    ax.set_title(xlabel)
                ax.set_xlabel(xlabel)
                ax.set_ylabel(
                    rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts")
                    if j == 0 else ("PDF" if density else "Counts")
                )
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_residuals_vs_p(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    species_suffix: str = "",
    p_edges: tuple[float, ...] = tuple(np.arange(0.0, 6.5, 0.5)),
    density: bool = True,
    out_path: str = "residuals_vs_p.pdf",
    title: str = r"Reconstruction residuals by $p$ bin (cut sample)",
):
    """2x4 grid; each cell overlays one residual per p slice (12 lines).

    `species_suffix`: "" = cut sample (default); "_Truth" = truth-matched pions.
    """
    bins = list(zip(p_edges[:-1], p_edges[1:]))
    colors = _bin_colors(len(bins))
    fig, axs = plt.subplots(
        len(species), len(RESIDUAL_VARS),
        figsize=(4 * len(RESIDUAL_VARS), 3.5 * len(species)),
        squeeze=False,
    )
    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            for j, (var, xlabel) in enumerate(RESIDUAL_VARS):
                ax = axs[i][j]
                h2 = f[f"Residual/{sp}{species_suffix}/{var}_vs_p"]
                slices = _project_2d_slices(
                    h2, list(bins), residual_axis="y", density=density,
                )
                for (p_lo, p_hi), color, (proj, edges) in zip(bins, colors, slices):
                    hep.histplot(
                        (proj, edges), ax=ax,
                        label=rf"${p_lo:.1f} \leq p < {p_hi:.1f}$",
                        color=color, histtype="step",
                    )
                if i == 0:
                    ax.set_title(xlabel)
                ax.set_xlabel(xlabel)
                ax.set_ylabel(
                    rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts")
                    if j == 0 else ("PDF" if density else "Counts")
                )
                ax.legend(fontsize=6, ncol=1, loc="best")
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_residuals_vs_theta(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    species_suffix: str = "",
    theta_min: float = 5.0,
    theta_max: float = 35.0,
    theta_step: float = 5.0,
    density: bool = True,
    out_path: str = "residuals_vs_theta.pdf",
    title: str = r"Reconstruction residuals by $\theta$ bin (cut sample)",
):
    """2x4 grid; each cell overlays one residual per theta slice (6 lines).

    `species_suffix`: "" = cut sample (default); "_Truth" = truth-matched pions.
    """
    edges = np.arange(theta_min, theta_max + 1e-9, theta_step)
    slices = list(zip(edges[:-1], edges[1:]))
    cmap = mpl.cm.tab10
    colors = [cmap(i % 10) for i in range(len(slices))]
    fig, axs = plt.subplots(
        len(species), len(RESIDUAL_VARS),
        figsize=(4 * len(RESIDUAL_VARS), 3.5 * len(species)),
        squeeze=False,
    )
    with uproot.open(file_path) as f:
        for i, sp in enumerate(species):
            for j, (var, xlabel) in enumerate(RESIDUAL_VARS):
                ax = axs[i][j]
                h2 = f[f"Residual/{sp}{species_suffix}/{var}_vs_theta"]
                projections = _project_2d_slices(
                    h2, list(slices), residual_axis="y",
                    slice_unit_hint="theta_deg", density=density,
                )
                for (t_lo, t_hi), color, (proj, edges_res) in zip(slices, colors, projections):
                    hep.histplot(
                        (proj, edges_res), ax=ax,
                        label=rf"${t_lo:.0f} \leq \theta < {t_hi:.0f}^\circ$",
                        color=color, histtype="step",
                    )
                if i == 0:
                    ax.set_title(xlabel)
                ax.set_xlabel(xlabel)
                ax.set_ylabel(
                    rf"{SPECIES_LABEL.get(sp, sp)}    " + ("PDF" if density else "Counts")
                    if j == 0 else ("PDF" if density else "Counts")
                )
                ax.legend(fontsize=6, ncol=1, loc="best")
    fig.suptitle(title)
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


# Display order in the PID composition plots. pid=0 is the sentinel for
# "no MC link" (no real MC::Particle row is ever written with pid=0).
# Any PID not listed here is bucketed into "other".
PID_CATEGORIES: tuple[tuple[int, str], ...] = (
    (0,     "no MC match"),
    (11,    r"$e^-$"),
    (-11,   r"$e^+$"),
    (13,    r"$\mu^-$"),
    (-13,   r"$\mu^+$"),
    (22,    r"$\gamma$"),
    (211,   r"$\pi^+$"),
    (-211,  r"$\pi^-$"),
    (321,   r"$K^+$"),
    (-321,  r"$K^-$"),
    (2212,  r"$p$"),
    (-2212, r"$\bar p$"),
    (2112,  r"$n$"),
)


def _bucket_pid_counts(h_pid) -> tuple[np.ndarray, list[str], float]:
    """Read a 1-D PID histogram into named counts + a single 'other' total.

    Hist bins are 1-unit wide centered on integer PIDs (see PidCompositionHists
    in CusnContaminationAlg.hpp), so the bin closest to each named PID is the
    count for that species.
    """
    values, edges = h_pid.to_numpy()
    centers = 0.5 * (edges[:-1] + edges[1:])
    labels = [lbl for _, lbl in PID_CATEGORIES]
    counts = np.zeros(len(PID_CATEGORIES))
    accounted = np.zeros(len(centers), dtype=bool)
    for i, (pid, _) in enumerate(PID_CATEGORIES):
        idx = int(np.argmin(np.abs(centers - pid)))
        counts[i] = values[idx]
        accounted[idx] = True
    other = float(values[~accounted].sum())
    return counts, labels, other


def plot_pid_composition_integrated(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    log_scale: bool = True,
    out_path: str = "pid_composition_integrated.pdf",
):
    """Per-species bar chart of the MC truth PID composition of the cut sample."""
    fig, axs = plt.subplots(1, len(species), figsize=(7 * len(species), 5), squeeze=False)
    axs = axs[0]
    with uproot.open(file_path) as f:
        for ax, sp in zip(axs, species):
            counts, labels, other = _bucket_pid_counts(f[f"PidContam/{sp}/mc_pid"])
            heights = np.append(counts, other)
            bar_labels = labels + ["other"]
            xs = np.arange(len(heights))
            ax.bar(xs, heights, color="tab:blue", edgecolor="black")
            ax.set_xticks(xs)
            ax.set_xticklabels(bar_labels, rotation=45, ha="right")
            total = float(heights.sum())
            ax.set_title(rf"{SPECIES_LABEL.get(sp, sp)} (total = {total:.0f})")
            ax.set_ylabel("Counts")
            if log_scale:
                ax.set_yscale("log")
            if total > 0:
                for x, hval in zip(xs, heights):
                    if hval > 0:
                        ax.text(
                            x, hval, f"{100 * hval / total:.2g}\\%",
                            ha="center", va="bottom", fontsize=7,
                        )
    fig.suptitle(r"MC truth PID composition of pion cut sample (integrated)")
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


def plot_pid_composition_vs_p(
    file_path: str = SIM_PATH,
    species: tuple[str, ...] = ("PiPlus", "PiMinus"),
    p_edges: tuple[float, ...] = tuple(np.arange(0.0, 10.0 + 1e-9, 0.5)),
    out_path: str = "pid_composition_vs_p.pdf",
):
    """Per-species line plot: MC truth PID fraction in the cut sample vs p.

    Exposes how the non-pion contamination evolves with momentum (low-p bins
    typically carry the most contamination).
    """
    p_arr = np.asarray(p_edges, dtype=float)
    p_centers = 0.5 * (p_arr[:-1] + p_arr[1:])
    n_p = len(p_centers)
    n_cat = len(PID_CATEGORIES) + 1   # +1 for "other"
    cmap = mpl.cm.tab20
    colors = [cmap(i % 20) for i in range(n_cat)]
    labels = [lbl for _, lbl in PID_CATEGORIES] + ["other"]

    fig, axs = plt.subplots(1, len(species), figsize=(7 * len(species), 5), squeeze=False)
    axs = axs[0]

    with uproot.open(file_path) as f:
        for ax, sp in zip(axs, species):
            cat_counts = np.zeros((n_cat, n_p))
            totals = np.zeros(n_p)
            for ip, (p_lo, p_hi) in enumerate(zip(p_arr[:-1], p_arr[1:])):
                h = f[f"PidContam/{sp}/mc_pid_p{_fmt_edge(p_lo)}_{_fmt_edge(p_hi)}"]
                counts, _, other = _bucket_pid_counts(h)
                cat_counts[:-1, ip] = counts
                cat_counts[-1, ip] = other
                totals[ip] = counts.sum() + other
            for ic, label in enumerate(labels):
                if cat_counts[ic].sum() == 0:
                    continue
                with np.errstate(divide="ignore", invalid="ignore"):
                    frac = np.where(totals > 0, cat_counts[ic] / totals, 0.0)
                ax.plot(p_centers, frac, marker="o", lw=1.2, ms=4,
                        label=label, color=colors[ic])
            ax.set_xlabel(r"$p$ [GeV/c]")
            ax.set_ylabel("Fraction of selected pions with given MC PID")
            ax.set_yscale("log")
            ax.set_ylim(1e-5, 2.0)
            ax.set_title(SPECIES_LABEL.get(sp, sp))
            ax.legend(fontsize=7, ncol=2, loc="lower right")
    fig.suptitle(r"MC truth PID fraction vs $p$ (cut sample)")
    fig.tight_layout()
    fig.savefig(out_path, bbox_inches="tight")


if __name__ == "__main__":
    main()
    plot_vz_theta_slices()
    plot_vz_theta_slices(
        species_suffix="_NoCuts",
        out_path="vz_Cu_vs_theta_slices_nocuts.pdf",
        title=r"Simulated $v_z$ in $\theta$ slices (no cuts, MC truth pid)",
    )
    plot_quality_sliced_vz()
    plot_vz_per_p_bin()
    plot_vz_per_p_bin(
        base_dir="Data",
        species_suffix="_NoCuts",
        out_path="vz_per_p_bin_nocuts.pdf",
        title=r"$v_z$ by momentum bin (no cuts, MC truth pid)",
    )
    plot_vz_three_way_integrated()
    plot_vz_three_way_p()
    plot_vz_three_way_theta()
    plot_vz_cut_steps_p()
    plot_vz_cut_steps_per_p()
    plot_vz_cut_steps_per_p(
        density=True,
        out_path="vz_cut_steps_per_p_overlay_pdf.pdf",
    )
    plot_vz_per_p_bin(
        species=("PiMinus",),
        species_suffix="_noPionCuts",
        out_path="vz_per_p_bin_piminus_pid_only.pdf",
        title=r"$\pi^-$ $v_z$ by momentum bin — forward, rec pid $= -211$ only (no other cuts)",
    )
    plot_residuals_integrated()
    plot_residuals_vs_p()
    plot_residuals_vs_theta()
    plot_residuals_integrated(
        species_suffix="_Truth",
        out_path="residuals_integrated_truth.pdf",
        title=r"Reconstruction residuals (integrated, truth pions any rec pid)",
    )
    plot_residuals_vs_p(
        species_suffix="_Truth",
        out_path="residuals_vs_p_truth.pdf",
        title=r"Reconstruction residuals by $p$ bin (truth pions any rec pid)",
    )
    plot_residuals_vs_theta(
        species_suffix="_Truth",
        out_path="residuals_vs_theta_truth.pdf",
        title=r"Reconstruction residuals by $\theta$ bin (truth pions any rec pid)",
    )
    plot_pid_composition_integrated()
    plot_pid_composition_vs_p()
