#!/usr/bin/env python

"""
SAGE Black Hole - Bulge Mass Relation Plot

This module generates a plot of the relationship between black hole mass and bulge mass from SAGE galaxy data.
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
    Create a black hole - bulge mass relation plot.

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

    # Maximum number of points to plot (for better performance and readability)
    dilute = 7500

    # Filter for valid galaxies with both bulge and black hole mass
    w = np.where((galaxies.BulgeMass > 0.01) & (galaxies.BlackHoleMass > 0.00001))[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with both bulge and black hole mass")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with both bulge and black hole mass",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BlackHoleBulgeRelation{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # If we have too many galaxies, randomly sample a subset
    if len(w) > dilute:
        w = random.sample(list(w), dilute)

    # Convert to physical units (Msun)
    bh_mass = np.log10(galaxies.BlackHoleMass[w] * 1.0e10 / hubble_h)
    bulge_mass = np.log10(galaxies.BulgeMass[w] * 1.0e10 / hubble_h)

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Black Hole-Bulge Relation plot debug:")
        print(f"  Number of galaxies plotted: {len(w)}")
        print(f"  Bulge mass range: {min(bulge_mass):.2f} to {max(bulge_mass):.2f}")
        print(f"  Black hole mass range: {min(bh_mass):.2f} to {max(bh_mass):.2f}")

    # Plot the galaxy data
    ax.scatter(
        bulge_mass, bh_mass, marker="o", s=1, c="k", alpha=0.5, label="Model galaxies"
    )

    # Add Häring & Rix 2004 observational relation
    # M_BH = 10^(8.2) * (M_bulge/10^11)^1.12
    x_hr = np.logspace(8, 12, 100)
    y_hr = 10 ** (8.2) * (x_hr / 1.0e11) ** 1.12

    ax.plot(np.log10(x_hr), np.log10(y_hr), "b-", label="Häring & Rix 2004", lw=2)

    # Customize the plot
    ax.set_xlabel(r"log$_{10}$ M$_{\rm bulge}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"log$_{10}$ M$_{\rm BH}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))

    # Set axis limits - matching the original plot
    ax.set_xlim(8.0, 12.0)
    ax.set_ylim(6.0, 10.0)

    # Add consistently styled legend
    setup_legend(ax, loc="upper left")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"BlackHoleBulgeRelation{output_format}")
    if verbose:
        print(f"Saving Black Hole - Bulge Mass Relation to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
