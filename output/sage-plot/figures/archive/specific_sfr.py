#!/usr/bin/env python

"""
SAGE Specific Star Formation Rate Plot

This module generates a specific star formation rate plot from SAGE galaxy data.
"""

import os
from random import sample, seed

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_ssfr_label,
    get_stellar_mass_label,
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
    dilute=7500,
    verbose=False,
):
    """
    Create a specific star formation rate plot.

    Args:
        galaxies: Galaxy data as a numpy recarray
        volume: Simulation volume in (Mpc/h)^3
        metadata: Dictionary with additional metadata
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output
        dilute: Maximum number of points to plot (for clarity)

    Returns:
        Path to the saved plot file
    """
    # Set random seed for reproducibility when diluting
    seed(2222)

    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Select galaxies with sufficient stellar mass
    w = np.where(galaxies.StellarMass > 0.01)[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with stellar mass > 0.01")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with stellar mass > 0.01",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"SpecificSFR{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Dilute the sample if needed
    if len(w) > dilute:
        w = sample(list(w), dilute)

    # Calculate stellar mass and specific SFR
    mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)
    sfr = galaxies.SfrDisk[w] + galaxies.SfrBulge[w]

    # Avoid log10(0) and division by zero
    valid_sfr = sfr > 0

    # Initialize ssfr with a very low value (below the plot range)
    ssfr = np.full_like(mass, -15.0)  # Well below typical plot range

    if np.any(valid_sfr):
        # Calculate SSFR only for galaxies with non-zero SFR
        stellar_mass_phys = galaxies.StellarMass[w][valid_sfr] * 1.0e10 / hubble_h
        ssfr[valid_sfr] = np.log10(sfr[valid_sfr] / stellar_mass_phys)

    # Plot the model galaxies
    ax.scatter(mass, ssfr, marker="o", s=1, c="k", alpha=0.5, label="Model galaxies")

    # Add a horizontal line at the division between star-forming and quiescent galaxies
    ssfr_cut = -11.0
    ax.axhline(y=ssfr_cut, c="r", ls="--", lw=2)
    # Use IN_FIGURE_TEXT_SIZE for consistent text sizing
    ax.text(11, ssfr_cut + 0.1, "Star-forming", color="b", fontsize=IN_FIGURE_TEXT_SIZE)
    ax.text(11, ssfr_cut - 0.5, "Quiescent", color="r", fontsize=IN_FIGURE_TEXT_SIZE)

    # Customize the plot
    ax.set_ylabel(get_ssfr_label(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_stellar_mass_label(), fontsize=AXIS_LABEL_SIZE)

    # Set the axis limits and minor ticks
    ax.set_xlim(8.0, 12.0)
    ax.set_ylim(-14.0, -8.0)
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))

    # Add consistently styled legend
    setup_legend(ax, loc="upper right")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"SpecificSFR{output_format}")
    if verbose:
        print(f"Saving Specific SFR to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
