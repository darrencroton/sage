#!/usr/bin/env python

"""
SAGE Stellar Mass Density Evolution Plot

This module generates a plot of the stellar mass density evolution from SAGE galaxy data.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_redshift_label,
    setup_legend,
    setup_plot_fonts,
)
from matplotlib.ticker import MultipleLocator


def plot(snapshots, params, output_dir="plots", output_format=".png", verbose=False):
    """
    Create a stellar mass density evolution plot.

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

    # Observational data from various sources
    # SMD observations from Marchesini+ 2009, h=0.7
    # Values are (minz, maxz, rho, -err, +err)

    # Dickenson 2003
    dickenson2003 = np.array(
        [
            (0.6, 1.4, 8.26, 0.08, 0.08),
            (1.4, 2.0, 7.86, 0.22, 0.33),
            (2.0, 2.5, 7.58, 0.29, 0.54),
            (2.5, 3.0, 7.52, 0.51, 0.48),
        ],
        dtype=np.float32,
    )

    # Drory 2005
    drory2005 = np.array(
        [
            (0.25, 0.75, 8.3, 0.15, 0.15),
            (0.75, 1.25, 8.16, 0.15, 0.15),
            (1.25, 1.75, 8.0, 0.16, 0.16),
            (1.75, 2.25, 7.85, 0.2, 0.2),
            (2.25, 3.0, 7.75, 0.2, 0.2),
            (3.0, 4.0, 7.58, 0.2, 0.2),
        ],
        dtype=np.float32,
    )

    # Perez-Gonzalez (2008)
    pg2008 = np.array(
        [
            (0.2, 0.4, 8.41, 0.06, 0.06),
            (0.4, 0.6, 8.37, 0.04, 0.04),
            (0.6, 0.8, 8.32, 0.05, 0.05),
            (0.8, 1.0, 8.24, 0.05, 0.05),
            (1.0, 1.3, 8.15, 0.05, 0.05),
            (1.3, 1.6, 7.95, 0.07, 0.07),
            (1.6, 2.0, 7.82, 0.07, 0.07),
            (2.0, 2.5, 7.67, 0.08, 0.08),
            (2.5, 3.0, 7.56, 0.18, 0.18),
            (3.0, 3.5, 7.43, 0.14, 0.14),
            (3.5, 4.0, 7.29, 0.13, 0.13),
        ],
        dtype=np.float32,
    )

    # Glazebrook 2004
    glazebrook2004 = np.array(
        [
            (0.8, 1.1, 7.98, 0.14, 0.1),
            (1.1, 1.3, 7.62, 0.14, 0.11),
            (1.3, 1.6, 7.9, 0.14, 0.14),
            (1.6, 2.0, 7.49, 0.14, 0.12),
        ],
        dtype=np.float32,
    )

    # Fontana 2006
    fontana2006 = np.array(
        [
            (0.4, 0.6, 8.26, 0.03, 0.03),
            (0.6, 0.8, 8.17, 0.02, 0.02),
            (0.8, 1.0, 8.09, 0.03, 0.03),
            (1.0, 1.3, 7.98, 0.02, 0.02),
            (1.3, 1.6, 7.87, 0.05, 0.05),
            (1.6, 2.0, 7.74, 0.04, 0.04),
            (2.0, 3.0, 7.48, 0.04, 0.04),
            (3.0, 4.0, 7.07, 0.15, 0.11),
        ],
        dtype=np.float32,
    )

    # Rudnick 2006
    rudnick2006 = np.array(
        [
            (0.0, 1.0, 8.17, 0.27, 0.05),
            (1.0, 1.6, 7.99, 0.32, 0.05),
            (1.6, 2.4, 7.88, 0.34, 0.09),
            (2.4, 3.2, 7.71, 0.43, 0.08),
        ],
        dtype=np.float32,
    )

    # Elsner 2008
    elsner2008 = np.array(
        [
            (0.25, 0.75, 8.37, 0.03, 0.03),
            (0.75, 1.25, 8.17, 0.02, 0.02),
            (1.25, 1.75, 8.02, 0.03, 0.03),
            (1.75, 2.25, 7.9, 0.04, 0.04),
            (2.25, 3.0, 7.73, 0.04, 0.04),
            (3.0, 4.0, 7.39, 0.05, 0.05),
        ],
        dtype=np.float32,
    )

    # Combine all observations
    obs = [
        dickenson2003,
        drory2005,
        pg2008,
        glazebrook2004,
        fontana2006,
        rudnick2006,
        elsner2008,
    ]

    # Plot all observations
    for o in obs:
        xval = ((o[:, 1] - o[:, 0]) / 2.0) + o[:, 0]
        if whichimf == 0:
            ax.errorbar(
                xval,
                np.log10(10 ** o[:, 2] * 1.6),
                xerr=(xval - o[:, 0], o[:, 1] - xval),
                yerr=(o[:, 3], o[:, 4]),
                alpha=0.3,
                lw=1.0,
                marker="o",
                ls="none",
            )
        elif whichimf == 1:
            ax.errorbar(
                xval,
                np.log10(10 ** o[:, 2] * 1.6 / 1.8),
                xerr=(xval - o[:, 0], o[:, 1] - xval),
                yerr=(o[:, 3], o[:, 4]),
                alpha=0.3,
                lw=1.0,
                marker="o",
                ls="none",
            )

    # Add a line for the legend
    ax.plot(
        [], [], color="k", alpha=0.3, marker="o", ls="none", label="Observational data"
    )

    # Check if we have any snapshots
    if len(snapshots) == 0:
        print("No snapshot data available for stellar mass density evolution plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No snapshot data available for stellar mass density evolution plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(
            output_dir, f"Stellar_Mass_Density_Evolution{output_format}"
        )
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Calculate stellar mass density for each snapshot
    smd = []
    redshifts = []

    for snap, (galaxies, volume, metadata) in snapshots.items():
        # Get the redshift for this snapshot
        redshift = metadata.get("redshift", 0.0)
        redshifts.append(redshift)

        # Extract hubble_h from metadata
        hubble_h = metadata.get("hubble_h", 0.73)

        # Skip if volume is zero (no valid data)
        if volume == 0:
            smd.append(0.0)
            continue

        # Select galaxies with reasonable stellar masses
        w = np.where(
            (galaxies.StellarMass / hubble_h > 0.01)
            & (galaxies.StellarMass / hubble_h < 1000.0)
        )[0]

        if len(w) > 0:
            # Sum stellar masses and normalize by volume
            # Need to convert to solar masses and account for volume units
            stellar_mass_sum = np.sum(galaxies.StellarMass[w]) * 1.0e10 / hubble_h
            density = stellar_mass_sum / (volume / hubble_h**3)
            smd.append(np.log10(density))
        else:
            smd.append(0.0)

    # Convert to numpy arrays
    redshifts = np.array(redshifts)
    smd = np.array(smd)

    # Sort by redshift
    sort_idx = np.argsort(redshifts)
    redshifts = redshifts[sort_idx]
    smd = smd[sort_idx]

    # Debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of snapshots: {len(snapshots)}")
        print(f"  Redshifts available: {redshifts}")
        print(f"  Stellar Mass Density values: {smd}")

    # Plot the model results
    nonzero = np.where(smd > 0.0)[0]
    if len(nonzero) > 0:
        if verbose:
            print(f"  Plotting {len(nonzero)} nonzero Stellar Mass Density points")
        ax.plot(redshifts[nonzero], smd[nonzero], "k-", lw=3.0, label="Model")
    else:
        # Always show warnings even without verbose
        print("  WARNING: No nonzero Stellar Mass Density points to plot!")

    # Customize the plot
    ax.set_ylabel(
        r"log$_{10}$ $\rho_{*}$ (M$_{\odot}$ Mpc$^{-3}$)", fontsize=AXIS_LABEL_SIZE
    )
    ax.set_xlabel(get_redshift_label(), fontsize=AXIS_LABEL_SIZE)

    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.1))

    ax.set_xlim(0.0, 8.0)
    ax.set_ylim(6.5, 9.0)

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

    output_path = os.path.join(
        output_dir, f"Stellar_Mass_Density_Evolution{output_format}"
    )
    if verbose:
        print(f"Saving Stellar Mass Density Evolution to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
