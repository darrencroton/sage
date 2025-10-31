#!/usr/bin/env python

"""
SAGE Halo Mass Function Plot

This module generates a halo mass function plot from SAGE halo data.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_mass_function_labels,
    get_halo_mass_label,
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
    Create a halo mass function plot.

    Args:
        galaxies: Galaxy data as a numpy recarray (containing halo information)
        volume: Simulation volume in (Mpc/h)^3
        metadata: Dictionary with additional metadata
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output

    Returns:
        Path to the saved plot file
    """
    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Set up binning
    binwidth = 0.1  # mass function histogram bin width

    # Prepare data - select halos (Type 0 = central galaxies = halos) with valid masses
    w = np.where((galaxies.Type == 0) & (galaxies.Mvir > 0.0))[0]

    # Check if we have any halos to plot
    if len(w) == 0:
        print("No halos found with Mvir > 0.0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No halos found with Mvir > 0.0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"HaloMassFunction{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Convert halo mass to log scale (Mvir is in units of 10^10 Msun/h)
    mass = np.log10(galaxies.Mvir[w] * 1.0e10 / hubble_h)

    # Set up histogram bins
    mi = np.floor(min(mass)) - 1
    ma = np.floor(max(mass)) + 1

    # Force some reasonable limits for halo masses
    mi = max(mi, 10.0)  # Don't go below 10^10 Msun
    ma = min(ma, 16.0)  # Don't go above 10^16 Msun

    nbins = int((ma - mi) / binwidth)

    # Calculate histogram for all halos
    counts, binedges = np.histogram(mass, range=(mi, ma), bins=nbins)
    xaxis = binedges[:-1] + 0.5 * binwidth

    # Print debugging info
    if verbose:
        print(f"  mi={mi}, ma={ma}, nbins={nbins}")
        print(f"  min mass={min(mass)}, max mass={max(mass)}")
        print(f"  volume={volume}, hubble_h={hubble_h}")
        print(f"  Number of halos: {len(w)}")

    # Plot the halo mass function
    ax.plot(
        xaxis,
        counts / volume * hubble_h * hubble_h * hubble_h / binwidth,
        "k-",
        lw=2,
        label="All Halos",
    )

    # Customize the plot
    ax.set_yscale("log")
    ax.set_xlim(10.0, 15.5)
    ax.set_ylim(1.0e-6, 1.0e-1)
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))

    # Set labels with larger font sizes
    ax.set_ylabel(get_mass_function_labels(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_halo_mass_label(), fontsize=AXIS_LABEL_SIZE)

    # Add consistently styled legend
    setup_legend(ax, loc="lower left")

    # Print debugging info for output directory
    if verbose:
        print(f"Output directory for HMF plot: {output_dir}")
        print(f"Output directory exists: {os.path.exists(output_dir)}")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"HaloMassFunction{output_format}")
    if verbose:
        print(f"Saving halo mass function to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
