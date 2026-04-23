#!/usr/bin/env python

"""
SAGE Black Hole Spin Distribution Plot

This module generates a histogram of black hole spin parameters (a*) from SAGE galaxy data,
showing the distribution of dimensionless spin values ranging from 0 (non-rotating) to 1
(maximally rotating Kerr black hole).
"""

import os

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
    Create a black hole spin distribution plot.

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
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Extract necessary metadata
    redshift = metadata.get("redshift", 0.0)

    # Filter for valid galaxies with black hole mass > 10^6 Msun
    # This avoids very small seed black holes and focuses on evolved BHs
    hubble_h = metadata["hubble_h"]
    min_bh_mass = 1e-4  # in 10^10 Msun/h units, equivalent to ~10^6 Msun
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
        output_path = os.path.join(output_dir, f"BlackHoleSpinDistribution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Extract spin values
    spins = galaxies.BlackHoleSpin[w]

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Black Hole Spin Distribution plot debug:")
        print(f"  Redshift: {redshift:.2f}")
        print(f"  Number of black holes: {len(w)}")
        print(f"  Spin range: {np.min(spins):.3f} to {np.max(spins):.3f}")
        print(f"  Mean spin: {np.mean(spins):.3f}")
        print(f"  Median spin: {np.median(spins):.3f}")

    # Create histogram
    bins = np.linspace(0, 1, 26)  # 25 bins covering [0, 1]
    counts, bin_edges = np.histogram(spins, bins=bins)

    # Normalize to get probability density
    bin_width = bin_edges[1] - bin_edges[0]
    density = counts / (len(spins) * bin_width)

    # Plot histogram as a bar chart
    bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2
    ax.bar(
        bin_centers,
        density,
        width=bin_width * 0.9,
        color="steelblue",
        alpha=0.7,
        edgecolor="black",
        linewidth=0.5,
        label=f"z={redshift:.1f}",
    )

    # Add reference lines for key spin values
    # a* = 0 (Schwarzschild)
    ax.axvline(0.0, color="red", linestyle="--", linewidth=1.5, alpha=0.7, label="Schwarzschild")

    # a* = 0.998 (near-maximal Kerr)
    ax.axvline(0.998, color="green", linestyle="--", linewidth=1.5, alpha=0.7, label="Near-maximal Kerr")

    # Add mean and median lines
    ax.axvline(
        np.mean(spins),
        color="darkblue",
        linestyle="-",
        linewidth=2,
        alpha=0.8,
        label=f"Mean: {np.mean(spins):.2f}",
    )
    ax.axvline(
        np.median(spins),
        color="purple",
        linestyle="-.",
        linewidth=2,
        alpha=0.8,
        label=f"Median: {np.median(spins):.2f}",
    )

    # Customize the plot
    ax.set_xlabel(r"Black Hole Spin Parameter $a_*$", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"Probability Density", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.05))
    ax.yaxis.set_minor_locator(MultipleLocator(0.1))

    # Set axis limits
    ax.set_xlim(0.0, 1.0)
    y_max = np.max(density) * 1.2 if np.max(density) > 0 else 1.0
    ax.set_ylim(0.0, y_max)

    # Add consistently styled legend
    setup_legend(ax, loc="upper left")

    # Add text annotation with statistics
    textstr = f"N = {len(w)}\nz = {redshift:.2f}"
    ax.text(
        0.95,
        0.95,
        textstr,
        transform=ax.transAxes,
        fontsize=IN_FIGURE_TEXT_SIZE,
        verticalalignment="top",
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

    output_path = os.path.join(output_dir, f"BlackHoleSpinDistribution{output_format}")
    if verbose:
        print(f"Saving Black Hole Spin Distribution to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
