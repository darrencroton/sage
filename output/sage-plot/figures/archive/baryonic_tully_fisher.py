#!/usr/bin/env python

"""
SAGE Baryonic Tully-Fisher Relationship Plot

This module generates a baryonic Tully-Fisher plot from SAGE galaxy data.
"""

import os
from random import sample, seed

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_baryonic_mass_label,
    get_vmax_label,
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
    Create a baryonic Tully-Fisher plot.

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

    # Select Sb/c galaxies (Type=0 and bulge/total ratio between 0.1 and 0.5)
    # First filter for non-zero stellar mass to avoid division by zero
    valid_mass = (
        (galaxies.Type == 0)
        & (galaxies.StellarMass > 0.0)
        & (galaxies.StellarMass + galaxies.ColdGas > 0.0)
    )

    # Then calculate ratios safely
    bulge_to_stellar = np.zeros_like(galaxies.StellarMass)
    bulge_to_stellar[valid_mass] = (
        galaxies.BulgeMass[valid_mass] / galaxies.StellarMass[valid_mass]
    )

    # Now apply all filters
    w = np.where(valid_mass & (bulge_to_stellar > 0.1) & (bulge_to_stellar < 0.5))[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No suitable galaxies found for Tully-Fisher plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No suitable galaxies found for Tully-Fisher plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BaryonicTullyFisher{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Dilute the sample if needed
    if len(w) > dilute:
        w = sample(list(w), dilute)

    # Calculate baryonic mass and max velocity
    mass = np.log10((galaxies.StellarMass[w] + galaxies.ColdGas[w]) * 1.0e10 / hubble_h)
    vel = np.log10(galaxies.Vmax[w])

    # Plot the model galaxies
    ax.scatter(
        vel, mass, marker="o", s=1, c="k", alpha=0.5, label="Model Sb/c galaxies"
    )

    # Plot Stark, McGaugh & Swatters 2009 relation
    w_obs = np.arange(0.5, 10.0, 0.5)
    TF = 3.94 * w_obs + 1.79
    ax.plot(w_obs, TF, "b-", lw=2.0, label="Stark, McGaugh \\& Swatters 2009")

    # Customize the plot
    ax.set_ylabel(get_baryonic_mass_label(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_vmax_label(), fontsize=AXIS_LABEL_SIZE)

    # Set the axis limits and minor ticks
    ax.set_xlim(1.4, 2.6)
    ax.set_ylim(8.0, 12.0)
    ax.xaxis.set_minor_locator(MultipleLocator(0.05))
    ax.yaxis.set_minor_locator(MultipleLocator(0.25))

    # Add consistently styled legend
    setup_legend(ax, loc="lower right")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"BaryonicTullyFisher{output_format}")
    if verbose:
        print(f"Saving Baryonic Tully-Fisher to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
