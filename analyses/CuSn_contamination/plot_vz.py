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
    cmap = mpl.cm.viridis
    colors = [cmap(i / max(1, len(bins) - 1)) for i in range(len(bins))]

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
    slice_unit_hint: str | None = None,
    density: bool = True,
) -> list[tuple[np.ndarray, np.ndarray]]:
    """Project a 2-D (binning x vz) uproot histogram onto vz for each slice.

    The vz axis is auto-detected by its negative range. When `slice_unit_hint`
    is "theta_deg" and the histogram's binning axis turns out to be in
    radians (max < 6.5), the slice edges are converted from deg to rad.
    """
    values, xedges, yedges = h2.to_numpy()
    x_is_vz = xedges.min() < -1.0
    y_is_vz = yedges.min() < -1.0
    if x_is_vz and not y_is_vz:
        bin_edges, vz_edges = yedges, xedges
        vals = values.T
    elif y_is_vz and not x_is_vz:
        bin_edges, vz_edges = xedges, yedges
        vals = values
    else:
        raise RuntimeError(
            f"Cannot identify vz axis: "
            f"xedges=[{xedges.min():.3g},{xedges.max():.3g}] "
            f"yedges=[{yedges.min():.3g},{yedges.max():.3g}]"
        )

    if slice_unit_hint == "theta_deg" and bin_edges.max() < 6.5:
        slice_edges = [(np.deg2rad(lo), np.deg2rad(hi)) for lo, hi in slice_edges]

    out: list[tuple[np.ndarray, np.ndarray]] = []
    widths = np.diff(vz_edges)
    for lo, hi in slice_edges:
        i_lo = np.searchsorted(bin_edges, lo, side="left")
        i_hi = np.searchsorted(bin_edges, hi, side="left")
        if i_hi <= i_lo:
            out.append((np.zeros(len(vz_edges) - 1), vz_edges))
            continue
        proj = vals[i_lo:i_hi, :].sum(axis=0)
        if density and proj.sum() > 0:
            proj = proj / (proj.sum() * widths)
        out.append((proj, vz_edges))
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
    cmap = mpl.cm.viridis
    colors = [cmap(i / max(1, len(bins) - 1)) for i in range(len(bins))]

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
