#!/usr/bin/env python3
"""Compare MC vs Data histograms from two separate CuSn contamination ROOT files."""

import sys
import uproot
import matplotlib.pyplot as plt
import numpy as np


def print_stats(label, f):
    print(f"\n=== {label} ===")
    print(f"{'Histogram':<40} {'Entries':>10} {'Mean':>10} {'StdDev':>10}")
    print("-" * 75)
    for key in sorted(f.keys()):
        h = f[key]
        if not hasattr(h, 'classname') or not h.classname.startswith("TH1"):
            continue
        values = h.values()
        edges = h.axis().edges()
        centers = (edges[:-1] + edges[1:]) / 2
        total = values.sum()
        if total > 0:
            mean = np.average(centers, weights=values)
            std = np.sqrt(np.average((centers - mean)**2, weights=values))
        else:
            mean, std = 0, 0
        print(f"{key:<40} {total:>10.0f} {mean:>10.3f} {std:>10.3f}")


def plot_comparison(mc_file, data_file):
    f_mc = uproot.open(mc_file)
    f_data = uproot.open(data_file)

    print_stats(f"MC: {mc_file}", f_mc)
    print_stats(f"Data: {data_file}", f_data)
    print()

    # Histogram pairs: (MC key from MC file, Data key from Data file, x-label)
    hist_pairs = [
        ("MC/Electron/p", "Data/Electron/p", "p [GeV]"),
        ("MC/Electron/phi", "Data/Electron/phi", r"$\phi$ [deg]"),
        ("MC/Electron/theta", "Data/Electron/theta", r"$\theta$ [deg]"),
        ("MC/Electron/vz", "Data/Electron/vz", "vz [cm]"),
        ("MC/Electron/vz_Cu", "Data/Electron/vz_Cu", "vz Cu [cm]"),
        ("MC/Electron/vz_Sn", "Data/Electron/vz_Sn", "vz Sn [cm]"),
        ("MC/Electron/vz_CuSn", "Data/Electron/vz_CuSn", "vz CuSn [cm]"),
    ]

    fig, axes = plt.subplots(3, 3, figsize=(15, 12))
    axes = axes.flatten()

    for i, (mc_key, data_key, xlabel) in enumerate(hist_pairs):
        ax = axes[i]

        # MC from MC file
        if mc_key in f_mc:
            h_mc = f_mc[mc_key]
            edges = h_mc.axis().edges()
            centers = (edges[:-1] + edges[1:]) / 2
            mc_vals = h_mc.values()
            mc_total = mc_vals.sum()
            if mc_total > 0:
                ax.step(centers, mc_vals / mc_total, where='mid',
                        label=f'MC ({mc_total:.0f})', color='blue', linewidth=1.5)

        # Data from Data file
        if data_key in f_data:
            h_data = f_data[data_key]
            edges = h_data.axis().edges()
            centers = (edges[:-1] + edges[1:]) / 2
            data_vals = h_data.values()
            data_total = data_vals.sum()
            if data_total > 0:
                ax.step(centers, data_vals / data_total, where='mid',
                        label=f'Data ({data_total:.0f})', color='red', linewidth=1.5)

        ax.set_xlabel(xlabel)
        ax.set_ylabel("Normalized")
        ax.legend(fontsize=8)
        ax.set_title(xlabel)

    for j in range(len(hist_pairs), len(axes)):
        axes[j].set_visible(False)

    plt.suptitle("CuSn Contamination: MC vs Data", fontsize=14)
    plt.tight_layout()
    outname = "mc_vs_data_comparison.png"
    plt.savefig(outname, dpi=150)
    print(f"Plot saved to: {outname}")
    plt.show()


if __name__ == "__main__":
    if len(sys.argv) == 3:
        plot_comparison(sys.argv[1], sys.argv[2])
    else:
        plot_comparison(
            "CuSn_contamination_CuSn_OB_sim.root",
            "CuSn_contamination_CuSn_OB_data.root",
        )
