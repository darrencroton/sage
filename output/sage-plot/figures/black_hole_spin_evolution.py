#!/usr/bin/env python

"""
SAGE Black Hole Spin Evolution Plot

This module generates a plot showing the evolution of black hole spin across cosmic time,
tracking the mean, median, and percentiles of the spin distribution as a function of redshift.
This reveals how spin demographics change as black holes grow through coherent accretion,
chaotic accretion, and mergers.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_redshift_label,
    setup_legend,
    setup_plot_fonts,
)
from matplotlib.ticker import MultipleLocator


def plot(snapshots, params, output_dir="plots", output_format=".png", verbose=False):
    """
    Create a black hole spin evolution plot.

    Args:
        snapshots: Dictionary mapping snapshot numbers to tuples of (galaxies, volume, metadata)
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output
        verbose: Enable verbose output

    Returns:
        Path to the saved plot file
    """
    # Set up the figure with two subplots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 10), sharex=True)

    # Apply consistent font settings
    setup_plot_fonts(ax1)
    setup_plot_fonts(ax2)

    # Minimum black hole mass for analysis (10^6 Msun in 10^10 Msun/h units)
    min_bh_mass = 1e-4

    # Debug information
    if verbose:
        print(f"Black Hole Spin Evolution plot debug:")
        print(f"  Number of snapshots: {len(snapshots)}")

    # Sort snapshots by redshift
    sorted_snapshots = [
        (snap, galaxies, volume, metadata)
        for snap, (galaxies, volume, metadata) in snapshots.items()
    ]
    sorted_snapshots.sort(key=lambda x: x[3]["redshift"], reverse=True)

    # Lists to store evolution data
    redshifts = []
    mean_spins = []
    median_spins = []
    percentile_25 = []
    percentile_75 = []
    percentile_10 = []
    percentile_90 = []
    num_bhs = []

    # Mass-binned spin evolution (for massive BHs)
    mean_spins_massive = []  # M_BH > 10^8 Msun
    median_spins_massive = []

    # Process each snapshot
    for snap, galaxies, volume, metadata in sorted_snapshots:
        redshift = metadata["redshift"]
        hubble_h = metadata["hubble_h"]

        # Filter for black holes above mass threshold
        w = np.where(galaxies.BlackHoleMass > min_bh_mass)[0]

        if len(w) > 10:  # Need at least 10 BHs for statistics
            spins = galaxies.BlackHoleSpin[w]

            redshifts.append(redshift)
            mean_spins.append(np.mean(spins))
            median_spins.append(np.median(spins))
            percentile_25.append(np.percentile(spins, 25))
            percentile_75.append(np.percentile(spins, 75))
            percentile_10.append(np.percentile(spins, 10))
            percentile_90.append(np.percentile(spins, 90))
            num_bhs.append(len(w))

            # Massive BHs (> 10^8 Msun)
            min_massive_bh = 1e-2  # 10^8 Msun in 10^10 Msun/h units
            w_massive = np.where(galaxies.BlackHoleMass > min_massive_bh)[0]
            if len(w_massive) > 10:
                spins_massive = galaxies.BlackHoleSpin[w_massive]
                mean_spins_massive.append(np.mean(spins_massive))
                median_spins_massive.append(np.median(spins_massive))
            else:
                mean_spins_massive.append(np.nan)
                median_spins_massive.append(np.nan)

            if verbose:
                print(f"  z={redshift:.2f}: {len(w)} BHs, mean spin={np.mean(spins):.3f}")

    # Convert to numpy arrays
    redshifts = np.array(redshifts)
    mean_spins = np.array(mean_spins)
    median_spins = np.array(median_spins)
    percentile_25 = np.array(percentile_25)
    percentile_75 = np.array(percentile_75)
    percentile_10 = np.array(percentile_10)
    percentile_90 = np.array(percentile_90)
    mean_spins_massive = np.array(mean_spins_massive)
    median_spins_massive = np.array(median_spins_massive)

    # Check if we have any data to plot
    if len(redshifts) == 0:
        print("No black hole data available for spin evolution plot")
        # Create empty plots with messages
        for ax in [ax1, ax2]:
            ax.text(
                0.5,
                0.5,
                "No black hole data available",
                horizontalalignment="center",
                verticalalignment="center",
                transform=ax.transAxes,
                fontsize=IN_FIGURE_TEXT_SIZE,
            )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BlackHoleSpinEvolution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # --- Top panel: Mean and median spin evolution ---

    # Plot percentile ranges
    ax1.fill_between(
        redshifts,
        percentile_10,
        percentile_90,
        color="lightblue",
        alpha=0.3,
        label="10-90th percentile (all BHs)",
    )
    ax1.fill_between(
        redshifts,
        percentile_25,
        percentile_75,
        color="steelblue",
        alpha=0.4,
        label="25-75th percentile (all BHs)",
    )

    # Plot mean and median for all BHs (M_BH > 10^6 Msun)
    ax1.plot(
        redshifts,
        mean_spins,
        "b-",
        linewidth=3,
        marker="o",
        markersize=6,
        label=r"Mean (M$_{\mathrm{BH}}$ > 10$^{6}$ M$_{\odot}$)",
    )
    ax1.plot(
        redshifts,
        median_spins,
        "b--",
        linewidth=2,
        marker="s",
        markersize=5,
        label=r"Median (M$_{\mathrm{BH}}$ > 10$^{6}$ M$_{\odot}$)",
    )

    # Plot mean and median for massive BHs (M_BH > 10^8 Msun)
    valid_massive = ~np.isnan(mean_spins_massive)
    if np.sum(valid_massive) > 0:
        ax1.plot(
            redshifts[valid_massive],
            mean_spins_massive[valid_massive],
            "r-",
            linewidth=3,
            marker="^",
            markersize=6,
            label=r"Mean (M$_{\mathrm{BH}}$ > 10$^{8}$ M$_{\odot}$)",
        )
        ax1.plot(
            redshifts[valid_massive],
            median_spins_massive[valid_massive],
            "r--",
            linewidth=2,
            marker="v",
            markersize=5,
            label=r"Median (M$_{\mathrm{BH}}$ > 10$^{8}$ M$_{\odot}$)",
        )

    # Add reference lines for key spin values
    ax1.axhline(0.2, color="gray", linestyle=":", linewidth=1, alpha=0.5, label="Seed BH spin")
    ax1.axhline(0.95, color="green", linestyle=":", linewidth=1, alpha=0.5, label="Target spin")

    # Customize top panel
    ax1.set_ylabel(r"Black Hole Spin Parameter $a_*$", fontsize=AXIS_LABEL_SIZE)
    ax1.yaxis.set_minor_locator(MultipleLocator(0.05))
    ax1.set_ylim(0.0, 1.0)
    setup_legend(ax1, loc="upper right", frameon=True)
    ax1.grid(True, alpha=0.3)

    # --- Bottom panel: Number of black holes ---

    ax2.semilogy(
        redshifts,
        num_bhs,
        "k-",
        linewidth=2,
        marker="o",
        markersize=5,
        label=r"N(M$_{\mathrm{BH}}$ > 10$^{6}$ M$_{\odot}$)",
    )

    # Customize bottom panel
    ax2.set_xlabel(get_redshift_label(), fontsize=AXIS_LABEL_SIZE)
    ax2.set_ylabel(r"Number of Black Holes", fontsize=AXIS_LABEL_SIZE)
    ax2.xaxis.set_minor_locator(MultipleLocator(0.5))
    setup_legend(ax2, loc="upper right")
    ax2.grid(True, alpha=0.3)

    # Set common x-axis limits
    if len(redshifts) > 0:
        z_min = max(0, np.min(redshifts) - 0.5)
        z_max = np.max(redshifts) + 0.5
        ax2.set_xlim(z_min, z_max)

    # Adjust layout to prevent overlap
    plt.tight_layout()

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"BlackHoleSpinEvolution{output_format}")
    if verbose:
        print(f"Saving Black Hole Spin Evolution to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
