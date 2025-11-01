#!/usr/bin/env python

"""
SAGE Stellar Mass Function Plot

This module generates a stellar mass function plot from SAGE galaxy data.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_mass_function_labels,
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
    Create a stellar mass function plot.

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
    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]

    # Get WhichIMF from the original allresults.py code or use IMF_Type if available
    whichimf = 1  # Default to Chabrier (allresults.py default)
    if params and "WhichIMF" in params:
        whichimf = int(params["WhichIMF"])
    elif params and "IMF_Type" in params:
        whichimf = int(params["IMF_Type"])

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Set up binning
    binwidth = 0.1  # mass function histogram bin width

    # Prepare data - exactly like original code
    w = np.where(galaxies.StellarMass > 0.0)[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with stellar mass > 0.0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with stellar mass > 0.0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"StellarMassFunction{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)

    # Calculate specific SFR for red/blue division
    sfr = galaxies.SfrDisk[w] + galaxies.SfrBulge[w]
    stellar_mass = galaxies.StellarMass[w] * 1.0e10 / hubble_h
    ssfr = sfr / stellar_mass
    ssfr_cut = -11.0  # log10(sSFR) cut between red and blue galaxies

    # Set up histogram bins - EXACTLY like original code
    # But with some safeguards for the test data
    mi = np.floor(min(mass)) - 2
    ma = np.floor(max(mass)) + 2

    # Force some reasonable limits for better consistency with original code
    mi = max(mi, 8.0)  # Don't go below 10^8 Msun
    ma = min(ma, 13.0)  # Don't go above 10^13 Msun

    nbins = int((ma - mi) / binwidth)

    # Calculate histogram for all galaxies
    counts, binedges = np.histogram(mass, range=(mi, ma), bins=nbins)
    xaxis = binedges[:-1] + 0.5 * binwidth

    # Print debugging info
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  mi={mi}, ma={ma}, nbins={nbins}")
        print(f"  min mass={min(mass)}, max mass={max(mass)}")
        print(f"  volume={volume}, hubble_h={hubble_h}")
        print(f"  whichimf={whichimf}")

    # Plot the main histogram - EXACTLY copying original code
    ax.plot(
        xaxis,
        counts / volume * hubble_h * hubble_h * hubble_h / binwidth,
        "k-",
        label="Model - All",
    )

    # Add red/blue separation - EXACTLY like original code
    # Red galaxies (passive)
    w_red = np.where(ssfr < 10.0**ssfr_cut)[0]
    mass_red = mass[w_red]
    counts_red, _ = np.histogram(mass_red, range=(mi, ma), bins=nbins)

    # Blue galaxies (star-forming)
    w_blue = np.where(ssfr >= 10.0**ssfr_cut)[0]
    mass_blue = mass[w_blue]
    counts_blue, _ = np.histogram(mass_blue, range=(mi, ma), bins=nbins)

    # Plot red/blue histograms - EXACTLY like original code
    ax.plot(
        xaxis,
        counts_red / volume * hubble_h * hubble_h * hubble_h / binwidth,
        "r:",
        lw=2,
        label="Model - Red",
    )
    ax.plot(
        xaxis,
        counts_blue / volume * hubble_h * hubble_h * hubble_h / binwidth,
        "b:",
        lw=2,
        label="Model - Blue",
    )

    # Add Baldry+2008 observational data
    baldry = np.array(
        [
            [7.05, 1.3531e-01, 6.0741e-02],
            [7.15, 1.3474e-01, 6.0109e-02],
            [7.25, 2.0971e-01, 7.7965e-02],
            [7.35, 1.7161e-01, 3.1841e-02],
            [7.45, 2.1648e-01, 5.7832e-02],
            [7.55, 2.1645e-01, 3.9988e-02],
            [7.65, 2.0837e-01, 4.8713e-02],
            [7.75, 2.0402e-01, 7.0061e-02],
            [7.85, 1.5536e-01, 3.9182e-02],
            [7.95, 1.5232e-01, 2.6824e-02],
            [8.05, 1.5067e-01, 4.8824e-02],
            [8.15, 1.3032e-01, 2.1892e-02],
            [8.25, 1.2545e-01, 3.5526e-02],
            [8.35, 9.8472e-02, 2.7181e-02],
            [8.45, 8.7194e-02, 2.8345e-02],
            [8.55, 7.0758e-02, 2.0808e-02],
            [8.65, 5.8190e-02, 1.3359e-02],
            [8.75, 5.6057e-02, 1.3512e-02],
            [8.85, 5.1380e-02, 1.2815e-02],
            [8.95, 4.4206e-02, 9.6866e-03],
            [9.05, 4.1149e-02, 1.0169e-02],
            [9.15, 3.4959e-02, 6.7898e-03],
            [9.25, 3.3111e-02, 8.3704e-03],
            [9.35, 3.0138e-02, 4.7741e-03],
            [9.45, 2.6692e-02, 5.5029e-03],
            [9.55, 2.4656e-02, 4.4359e-03],
            [9.65, 2.2885e-02, 3.7915e-03],
            [9.75, 2.1849e-02, 3.9812e-03],
            [9.85, 2.0383e-02, 3.2930e-03],
            [9.95, 1.9929e-02, 2.9370e-03],
            [10.05, 1.8865e-02, 2.4624e-03],
            [10.15, 1.8136e-02, 2.5208e-03],
            [10.25, 1.7657e-02, 2.4217e-03],
            [10.35, 1.6616e-02, 2.2784e-03],
            [10.45, 1.6114e-02, 2.1783e-03],
            [10.55, 1.4366e-02, 1.8819e-03],
            [10.65, 1.2588e-02, 1.8249e-03],
            [10.75, 1.1372e-02, 1.4436e-03],
            [10.85, 9.1213e-03, 1.5816e-03],
            [10.95, 6.1125e-03, 9.6735e-04],
            [11.05, 4.3923e-03, 9.6254e-04],
            [11.15, 2.5463e-03, 5.0038e-04],
            [11.25, 1.4298e-03, 4.2816e-04],
            [11.35, 6.4867e-04, 1.6439e-04],
            [11.45, 2.8294e-04, 9.9799e-05],
            [11.55, 1.0617e-04, 4.9085e-05],
            [11.65, 3.2702e-05, 2.4546e-05],
            [11.75, 1.2571e-05, 1.2571e-05],
            [11.85, 8.4589e-06, 8.4589e-06],
            [11.95, 7.4764e-06, 7.4764e-06],
        ],
        dtype=np.float32,
    )

    # Adjust Baldry data for IMF and h - EXACTLY matching original code in allresults.py
    baldry_xval = np.log10(10 ** baldry[:, 0] / hubble_h / hubble_h)
    if whichimf == 1:  # Chabrier IMF
        baldry_xval = baldry_xval - 0.26  # Convert from Salpeter to Chabrier IMF

    baldry_yvalU = (baldry[:, 1] + baldry[:, 2]) * hubble_h * hubble_h * hubble_h
    baldry_yvalL = (baldry[:, 1] - baldry[:, 2]) * hubble_h * hubble_h * hubble_h

    # Plot the observational data - EXACTLY as in original code
    ax.fill_between(
        baldry_xval, baldry_yvalU, baldry_yvalL, facecolor="purple", alpha=0.25
    )

    # This next line is exactly as the original code does it to get the legend right
    # The original code seems to add this separate line for the legend entry
    # This line is exactly as the original code does it to get the legend right
    # First create an empty plot with the right style for the legend
    ax.plot([], [], color="purple", alpha=0.3, lw=8, label="Baldry et al. 2008 (z=0.1)")

    # Customize the plot
    ax.set_yscale("log")
    ax.set_xlim(8.0, 12.5)
    ax.set_ylim(1.0e-6, 1.0e-1)
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))

    # Set labels with larger font sizes
    ax.set_ylabel(get_mass_function_labels(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_stellar_mass_label(), fontsize=AXIS_LABEL_SIZE)

    # Add consistently styled legend
    setup_legend(ax, loc="lower left")

    # Print debugging info for output directory
    if verbose:
        print(f"Output directory for SMF plot: {output_dir}")
        print(f"Output directory exists: {os.path.exists(output_dir)}")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"StellarMassFunction{output_format}")
    if verbose:
        print(f"Saving stellar mass function to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
