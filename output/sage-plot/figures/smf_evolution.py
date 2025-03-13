#!/usr/bin/env python

"""
SAGE Stellar Mass Function Evolution Plot

This module generates a stellar mass function evolution plot from SAGE galaxy data.
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


def plot(snapshots, params, output_dir="plots", output_format=".png", verbose=False):
    """
    Create a stellar mass function evolution plot.

    Args:
        snapshots: Dictionary mapping snapshot numbers to tuples of (galaxies, volume, metadata)
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output

    Returns:
        Path to the saved plot file
    """
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Determine IMF type from params
    whichimf = params.get("IMF_Type", 1)  # Default to Chabrier IMF

    # Set up binning
    binwidth = 0.1

    # Add Marchesini et al. 2009 observational data (z=[0.1])
    M = np.arange(7.0, 11.8, 0.01)
    Mstar = np.log10(10.0**10.96)
    alpha = -1.18
    phistar = 30.87 * 1e-4
    xval = 10.0 ** (M - Mstar)
    yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)

    if whichimf == 0:
        ax.plot(
            np.log10(10.0**M * 1.6),
            yval,
            ":",
            lw=10,
            alpha=0.5,
            label="Marchesini et al. 2009 z=[0.1]",
        )
    elif whichimf == 1:
        ax.plot(
            np.log10(10.0**M * 1.6 / 1.8),
            yval,
            ":",
            lw=10,
            alpha=0.5,
            label="Marchesini et al. 2009 z=[0.1]",
        )

    # Add Marchesini et al. 2009 observational data (z=[1.3,2.0])
    M = np.arange(9.3, 11.8, 0.01)
    Mstar = np.log10(10.0**10.91)
    alpha = -0.99
    phistar = 10.17 * 1e-4
    xval = 10.0 ** (M - Mstar)
    yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)

    if whichimf == 0:
        ax.plot(
            np.log10(10.0**M * 1.6),
            yval,
            "b:",
            lw=10,
            alpha=0.5,
            label="... z=[1.3,2.0]",
        )
    elif whichimf == 1:
        ax.plot(
            np.log10(10.0**M * 1.6 / 1.8),
            yval,
            "b:",
            lw=10,
            alpha=0.5,
            label="... z=[1.3,2.0]",
        )

    # Add Marchesini et al. 2009 observational data (z=[2.0,3.0])
    M = np.arange(9.7, 11.8, 0.01)
    Mstar = np.log10(10.0**10.96)
    alpha = -1.01
    phistar = 3.95 * 1e-4
    xval = 10.0 ** (M - Mstar)
    yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)

    if whichimf == 0:
        ax.plot(
            np.log10(10.0**M * 1.6),
            yval,
            "g:",
            lw=10,
            alpha=0.5,
            label="... z=[2.0,3.0]",
        )
    elif whichimf == 1:
        ax.plot(
            np.log10(10.0**M * 1.6 / 1.8),
            yval,
            "g:",
            lw=10,
            alpha=0.5,
            label="... z=[2.0,3.0]",
        )

    # Add Marchesini et al. 2009 observational data (z=[3.0,4.0])
    M = np.arange(10.0, 11.8, 0.01)
    Mstar = np.log10(10.0**11.38)
    alpha = -1.39
    phistar = 0.53 * 1e-4
    xval = 10.0 ** (M - Mstar)
    yval = np.log(10.0) * phistar * xval ** (alpha + 1) * np.exp(-xval)

    if whichimf == 0:
        ax.plot(
            np.log10(10.0**M * 1.6),
            yval,
            "r:",
            lw=10,
            alpha=0.5,
            label="... z=[3.0,4.0]",
        )
    elif whichimf == 1:
        ax.plot(
            np.log10(10.0**M * 1.6 / 1.8),
            yval,
            "r:",
            lw=10,
            alpha=0.5,
            label="... z=[3.0,4.0]",
        )

    # Debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of snapshots: {len(snapshots)}")
        for snap, (galaxies, volume, metadata) in snapshots.items():
            print(
                f"  Snapshot {snap}: {len(galaxies)} galaxies, z={metadata.get('redshift', 'unknown')}"
            )

    # Sort snapshots by redshift
    sorted_snapshots = [
        (snap, galaxies, volume, metadata)
        for snap, (galaxies, volume, metadata) in snapshots.items()
    ]
    # Sort by redshift
    sorted_snapshots.sort(key=lambda x: x[3]["redshift"])

    # Colors for different redshifts
    colors = ["k", "b", "g", "r", "m", "y", "c", "orange"]

    # Check if we have any snapshots
    if len(sorted_snapshots) == 0:
        print("No snapshot data available for SMF evolution plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No snapshot data available for SMF evolution plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(
            output_dir, f"StellarMassFunction_Evolution{output_format}"
        )
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Plot model SMFs at various redshifts
    for i, (snap, galaxies, volume, metadata) in enumerate(
        sorted_snapshots[:8]
    ):  # Limit to 8 redshifts
        hubble_h = metadata["hubble_h"]
        redshift = metadata["redshift"]
        color = colors[i % len(colors)]

        # Debug output - only show if verbose is enabled
        if verbose:
            print(f"Processing SMF for snapshot {snap}, z={redshift:.1f}")

        # Select galaxies with stellar mass
        w = np.where(galaxies.StellarMass > 0.0)[0]

        # Skip this snapshot if no galaxies found
        if len(w) == 0:
            print(f"No galaxies found for z={redshift:.1f}")
            continue

        mass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)

        # Set up histogram bins
        mi = np.floor(min(mass)) - 2
        ma = np.floor(max(mass)) + 2
        nbins = int((ma - mi) / binwidth)

        # Calculate histogram
        counts, binedges = np.histogram(mass, range=(mi, ma), bins=nbins)
        xaxis = binedges[:-1] + 0.5 * binwidth

        # Plot the histogram
        label = f"z={redshift:.1f}" if i == 0 else None
        ax.plot(
            xaxis, counts / volume * hubble_h**3 / binwidth, color=color, linestyle="-"
        )

        # Add a text label near the line
        if i < 4:  # Only label the first 4 redshifts to avoid clutter
            # Find a suitable y-value for the text
            idx = np.argmin(np.abs(xaxis - 10.5))
            y_value = counts[idx] / volume * hubble_h**3 / binwidth
            ax.text(
                10.7,
                y_value,
                f"z={redshift:.1f}",
                color=color,
                fontsize=IN_FIGURE_TEXT_SIZE,
            )

    # Customize the plot
    ax.set_yscale("log")
    ax.set_xlim(8.0, 12.5)
    ax.set_ylim(1.0e-6, 1.0e-1)
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))

    ax.set_ylabel(get_mass_function_labels(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_stellar_mass_label(), fontsize=AXIS_LABEL_SIZE)

    # Add consistently styled legend
    setup_legend(ax, loc="lower left")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(
        output_dir, f"StellarMassFunction_Evolution{output_format}"
    )
    if verbose:
        print(f"Saving Stellar Mass Function Evolution to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
