#!/usr/bin/env python

"""
SAGE Black Hole Spin - Mass Relation Plot

This module generates a scatter plot showing the relationship between black hole spin (a*)
and black hole mass. This reveals how spin evolves as black holes grow through accretion
and mergers.
"""

import os
import random

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    setup_legend,
    setup_plot_fonts,
)
from matplotlib.ticker import MultipleLocator


def plot(
    galaxies,
    volume,
    metadata,
    params,
    output_dir="plots",
    output_format=".png",
    verbose=False,
):
    """
    Create a black hole spin - mass relation plot.

    Args:
        galaxies: Galaxy data as a numpy recarray
        volume: Simulation volume in (Mpc/h)^3
        metadata: Dictionary with additional metadata
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output
        verbose: Enable verbose output

    Returns:
        Path to the saved plot file
    """
    # Set random seed for reproducibility when sampling points
    random.seed(42)

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]
    redshift = metadata.get("redshift", 0.0)

    # Maximum number of points to plot (for better performance and readability)
    dilute = 5000

    # Filter for valid galaxies with black hole mass > 10^6 Msun
    min_bh_mass = 1e-4  # in 10^10 Msun/h units
    w = np.where(galaxies.BlackHoleMass > min_bh_mass)[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print(f"No black holes found with mass > 10^6 Msun at z={redshift:.2f}")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            f"No black holes found with mass > 10^6 Msun\nat z={redshift:.2f}",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BlackHoleSpinMassRelation{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # If we have too many galaxies, randomly sample a subset
    if len(w) > dilute:
        w = random.sample(list(w), dilute)

    # Convert to physical units (Msun)
    bh_mass = np.log10(galaxies.BlackHoleMass[w] * 1.0e10 / hubble_h)
    spins = galaxies.BlackHoleSpin[w]

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Black Hole Spin-Mass Relation plot debug:")
        print(f"  Redshift: {redshift:.2f}")
        print(f"  Number of black holes plotted: {len(w)}")
        print(f"  BH mass range: {np.min(bh_mass):.2f} to {np.max(bh_mass):.2f}")
        print(f"  Spin range: {np.min(spins):.3f} to {np.max(spins):.3f}")

    # Create color map based on halo mass (if available)
    if hasattr(galaxies, "Mvir"):
        halo_mass = np.log10(galaxies.Mvir[w] * 1.0e10 / hubble_h)
        # Use halo mass for coloring
        scatter = ax.scatter(
            bh_mass,
            spins,
            c=halo_mass,
            cmap="viridis",
            marker="o",
            s=10,
            alpha=0.5,
            edgecolors="none",
        )
        # Add colorbar
        cbar = plt.colorbar(scatter, ax=ax, label=r"log$_{10}$ M$_{\rm halo}$ (M$_{\odot}$)")
        cbar.ax.tick_params(labelsize=LEGEND_FONT_SIZE)
    else:
        # Simple scatter plot without color coding
        ax.scatter(
            bh_mass, spins, marker="o", s=10, c="steelblue", alpha=0.5, edgecolors="none"
        )

    # Calculate and plot running median in mass bins
    mass_bins = np.linspace(np.min(bh_mass), np.max(bh_mass), 15)
    bin_centers = []
    median_spins = []
    percentile_25 = []
    percentile_75 = []

    for i in range(len(mass_bins) - 1):
        in_bin = (bh_mass >= mass_bins[i]) & (bh_mass < mass_bins[i + 1])
        if np.sum(in_bin) > 10:  # Need at least 10 BHs in bin
            bin_centers.append((mass_bins[i] + mass_bins[i + 1]) / 2)
            median_spins.append(np.median(spins[in_bin]))
            percentile_25.append(np.percentile(spins[in_bin], 25))
            percentile_75.append(np.percentile(spins[in_bin], 75))

    if len(bin_centers) > 0:
        bin_centers = np.array(bin_centers)
        median_spins = np.array(median_spins)
        percentile_25 = np.array(percentile_25)
        percentile_75 = np.array(percentile_75)

        # Plot median line
        ax.plot(
            bin_centers,
            median_spins,
            "r-",
            linewidth=3,
            label="Median",
            zorder=10,
        )

        # Plot 25th-75th percentile range as shaded region
        ax.fill_between(
            bin_centers,
            percentile_25,
            percentile_75,
            color="red",
            alpha=0.2,
            label="25-75th percentile",
            zorder=5,
        )

    # Add reference lines for key spin values
    ax.axhline(0.0, color="gray", linestyle=":", linewidth=1, alpha=0.5)
    ax.axhline(0.95, color="gray", linestyle=":", linewidth=1, alpha=0.5)

    # Customize the plot
    ax.set_xlabel(r"log$_{10}$ M$_{\rm BH}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"Black Hole Spin Parameter $a_*$", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.1))

    # Set axis limits
    ax.set_ylim(0.0, 1.0)

    # Add consistently styled legend
    setup_legend(ax, loc="upper left")

    # Add text annotation with redshift
    textstr = f"z = {redshift:.2f}\nN = {len(w)}"
    ax.text(
        0.95,
        0.05,
        textstr,
        transform=ax.transAxes,
        fontsize=IN_FIGURE_TEXT_SIZE,
        verticalalignment="bottom",
        horizontalalignment="right",
        bbox=dict(boxstyle="round", facecolor="wheat", alpha=0.3),
    )

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"BlackHoleSpinMassRelation{output_format}")
    if verbose:
        print(f"Saving Black Hole Spin-Mass Relation to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
