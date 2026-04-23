#!/usr/bin/env python

"""
SAGE Stellar Mass - Halo Mass Relation Plot

This module generates a plot of the relationship between stellar mass and halo mass from SAGE galaxy data.
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
    Create a stellar mass - halo mass relation plot.

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

    # Filter for valid galaxies with both halo and stellar mass
    w = np.where((galaxies.Mvir > 0) & (galaxies.StellarMass > 0))[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with both halo and stellar mass")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with both halo and stellar mass",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"StellarHaloMassRelation{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # If we have too many galaxies, randomly sample a subset
    if len(w) > dilute:
        w = random.sample(list(w), dilute)

    # Convert to physical units (Msun)
    stellar_mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)
    halo_mass = np.log10(galaxies.Mvir[w] * 1.0e10 / hubble_h)

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Stellar Mass - Halo Mass Relation plot debug:")
        print(f"  Number of galaxies plotted: {len(w)}")
        print(f"  Halo mass range: {min(halo_mass):.2f} to {max(halo_mass):.2f}")
        print(f"  Stellar mass range: {min(stellar_mass):.2f} to {max(stellar_mass):.2f}")

    # Plot the galaxy data
    ax.scatter(
        halo_mass, stellar_mass, marker="o", s=1, c="k", alpha=0.5, label="Model galaxies"
    )

    # Add Moster et al. (2010) observational relation
    # M_star / M_halo = 2 * N * ( (M_halo/M_1)^-beta + (M_halo/M_1)^gamma )^-1
    log10_M1 = 11.88
    N = 0.0351
    beta = 1.376
    gamma = 0.608
    
    M_1 = 10**log10_M1
    M_halo = np.logspace(10, 15, 100)
    
    M_star_by_M_halo = 2 * N * ((M_halo / M_1)**-beta + (M_halo / M_1)**gamma)**-1
    M_star = M_star_by_M_halo * M_halo

    ax.plot(np.log10(M_halo), np.log10(M_star), "b-", label="Moster et al. 2010", lw=2)

    # Customize the plot
    ax.set_xlabel(r"log$_{10}$ M$_{\rm halo}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"log$_{10}$ M$_{\rm stellar}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))

    # Set axis limits
    ax.set_xlim(10.0, 15.0)
    ax.set_ylim(8.0, 12.5)

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

    output_path = os.path.join(output_dir, f"StellarHaloMassRelation{output_format}")
    if verbose:
        print(f"Saving Stellar Mass - Halo Mass Relation to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
