#!/usr/bin/env python

"""
SAGE Mass Reservoir Scatter Plot

This module generates a scatter plot showing the mass in different galaxy components vs. halo mass.
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
    Create a mass reservoir scatter plot.

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

    # Filter for type 0 (central) galaxies with non-zero Mvir
    w = np.where(
        (galaxies.Type == 0) & (galaxies.Mvir > 1.0) & (galaxies.StellarMass > 0.0)
    )[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No central galaxies found with Mvir > 1.0 and StellarMass > 0.0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No central galaxies found with Mvir > 1.0 and StellarMass > 0.0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"MassReservoirScatter{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # If we have too many galaxies, randomly sample a subset
    if len(w) > dilute:
        w = random.sample(list(w), dilute)

    # Get halo mass in log10 Msun units
    mvir = np.log10(galaxies.Mvir[w] * 1.0e10)

    # Get component masses in log10 Msun units
    stellar_mass = np.log10(galaxies.StellarMass[w] * 1.0e10)
    cold_gas = np.log10(np.maximum(galaxies.ColdGas[w] * 1.0e10, 1.0))  # Avoid log(0)
    hot_gas = np.log10(np.maximum(galaxies.HotGas[w] * 1.0e10, 1.0))
    ejected_gas = np.log10(np.maximum(galaxies.EjectedMass[w] * 1.0e10, 1.0))
    ics = np.log10(np.maximum(galaxies.IntraClusterStars[w] * 1.0e10, 1.0))

    # Print some debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of galaxies plotted: {len(w)}")
        print(f"  Halo mass range: {min(mvir):.2f} to {max(mvir):.2f}")
        print(
            f"  Stellar mass range: {min(stellar_mass):.2f} to {max(stellar_mass):.2f}"
        )

    # Plot each mass component
    ax.scatter(mvir, stellar_mass, marker="o", s=0.8, c="k", alpha=0.5, label="Stars")
    ax.scatter(mvir, cold_gas, marker="o", s=0.8, c="blue", alpha=0.5, label="Cold gas")
    ax.scatter(mvir, hot_gas, marker="o", s=0.8, c="red", alpha=0.5, label="Hot gas")
    ax.scatter(
        mvir, ejected_gas, marker="o", s=0.8, c="green", alpha=0.5, label="Ejected gas"
    )
    ax.scatter(
        mvir, ics, marker="x", s=5, c="yellow", alpha=0.7, label="Intracluster stars"
    )

    # Customize the plot
    ax.set_xlabel(r"log M$_{\rm vir}$ (h$^{-1}$ M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"Stellar, cold, hot, ejected, ICS mass", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))

    # Set axis limits - matching the original plot
    x_min = max(10.0, min(mvir) - 0.5)
    x_max = min(14.0, max(mvir) + 0.5)
    y_min = max(7.5, min(min(stellar_mass), min(cold_gas), min(hot_gas)) - 0.5)
    y_max = min(12.5, max(max(stellar_mass), max(cold_gas), max(hot_gas)) + 0.5)

    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)

    # Add text annotation 'All' in the bottom-right corner
    ax.text(
        0.95, 0.05, r"All", transform=ax.transAxes, fontsize=12, ha="right", va="bottom"
    )

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

    output_path = os.path.join(output_dir, f"MassReservoirScatter{output_format}")
    if verbose:
        print(f"Saving Mass Reservoir Scatter plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
