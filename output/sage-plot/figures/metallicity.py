#!/usr/bin/env python

"""
SAGE Metallicity Plot

This module generates a plot showing the gas-phase metallicity vs. stellar mass for SAGE galaxy data.
"""

import os
import random

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
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
    verbose=False,
):
    """
    Create a metallicity vs. stellar mass plot.

    Args:
        galaxies: Galaxy data as a numpy recarray
        volume: Simulation volume in (Mpc/h)^3
        metadata: Dictionary with additional metadata
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output

    Returns:
        Path to the saved plot file
    """
    # Set random seed for reproducibility when sampling points
    random.seed(2222)

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]

    # Get WhichIMF from parameters
    whichimf = 1  # Default to Chabrier IMF
    if params:
        if "WhichIMF" in params:
            whichimf = int(params["WhichIMF"])
        elif "IMF_Type" in params:
            whichimf = int(params["IMF_Type"])

    # Maximum number of points to plot (for better performance and readability)
    dilute = 7500

    # First filter for valid mass values - avoid division by zero
    valid_mass = (
        (galaxies.Type == 0)
        & (galaxies.StellarMass > 0.01)
        & (galaxies.ColdGas > 0.0)
        & (galaxies.MetalsColdGas > 0.0)
    )

    # Calculate gas fraction safely for valid galaxies
    gas_fraction = np.zeros_like(galaxies.StellarMass)
    gas_fraction[valid_mass] = galaxies.ColdGas[valid_mass] / (
        galaxies.StellarMass[valid_mass] + galaxies.ColdGas[valid_mass]
    )

    # Now apply all filters
    w = np.where(valid_mass & (gas_fraction > 0.1))[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No suitable galaxies found for metallicity plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No suitable galaxies found for metallicity plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"Metallicity{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # If we have too many galaxies, randomly sample a subset
    if len(w) > dilute:
        w = random.sample(list(w), dilute)

    # Calculate metallicity (12 + log10[O/H]) and convert stellar mass to log scale
    stellar_mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)
    # Metallicity in units of solar (Z_solar = 0.02)
    metallicity = (
        np.log10((galaxies.MetalsColdGas[w] / galaxies.ColdGas[w]) / 0.02) + 9.0
    )

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Metallicity plot debug:")
        print(f"  Number of galaxies plotted: {len(w)}")
        print(
            f"  Stellar mass range: {min(stellar_mass):.2f} to {max(stellar_mass):.2f}"
        )
        print(f"  Metallicity range: {min(metallicity):.3f} to {max(metallicity):.3f}")
        print(f"  WhichIMF: {whichimf}")

    # Plot the galaxy data
    ax.scatter(
        stellar_mass,
        metallicity,
        marker="o",
        s=1,
        c="k",
        alpha=0.5,
        label="Model galaxies",
    )

    # Add Tremonti et al. 2003 observational relation
    # Relation: 12 + log(O/H) = -1.492 + 1.847*log(M*) - 0.08026*log(M*)^2
    # Original relation is for Kroupa IMF, need to convert
    mass_range = np.arange(7.0, 13.0, 0.1)
    tremonti_Z = -1.492 + 1.847 * mass_range - 0.08026 * mass_range * mass_range

    if whichimf == 0:
        # Convert from Kroupa IMF to Salpeter IMF (+0.176 dex)
        adjusted_mass = np.log10((10**mass_range) * 1.5)
        ax.plot(adjusted_mass, tremonti_Z, "b-", lw=2.0, label="Tremonti et al. 2003")
    elif whichimf == 1:
        # Convert from Kroupa IMF to Chabrier IMF (+0.176 dex, then -0.26 dex)
        adjusted_mass = np.log10((10**mass_range) * 1.5 / 1.8)
        ax.plot(adjusted_mass, tremonti_Z, "b-", lw=2.0, label="Tremonti et al. 2003")

    # Customize the plot
    ax.set_xlabel(get_stellar_mass_label(), fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"12 + log$_{10}$[O/H]", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.1))

    # Set axis limits - matching the original plot
    ax.set_xlim(8.0, 12.0)
    ax.set_ylim(8.0, 9.5)

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

    output_path = os.path.join(output_dir, f"Metallicity{output_format}")
    if verbose:
        print(f"Saving Metallicity plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
