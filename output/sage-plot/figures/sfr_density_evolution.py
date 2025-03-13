#!/usr/bin/env python

"""
SAGE Star Formation Rate Density Evolution Plot

This module generates a plot of the star formation rate density evolution from SAGE galaxy data.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_redshift_label,
    get_sfr_density_label,
    setup_legend,
    setup_plot_fonts,
)
from matplotlib.ticker import MultipleLocator


def plot(snapshots, params, output_dir="plots", output_format=".png", verbose=False):
    """
    Create a star formation rate density evolution plot.

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

    # Add observational data (compilation used in many papers)
    ObsSFRdensity = np.array(
        [
            [0, 0.0158489, 0, 0, 0.0251189, 0.01000000],
            [0.150000, 0.0173780, 0, 0.300000, 0.0181970, 0.0165959],
            [0.0425000, 0.0239883, 0.0425000, 0.0425000, 0.0269153, 0.0213796],
            [0.200000, 0.0295121, 0.100000, 0.300000, 0.0323594, 0.0269154],
            [0.350000, 0.0147911, 0.200000, 0.500000, 0.0173780, 0.0125893],
            [0.625000, 0.0275423, 0.500000, 0.750000, 0.0331131, 0.0229087],
            [0.825000, 0.0549541, 0.750000, 1.00000, 0.0776247, 0.0389045],
            [0.625000, 0.0794328, 0.500000, 0.750000, 0.0954993, 0.0660693],
            [0.700000, 0.0323594, 0.575000, 0.825000, 0.0371535, 0.0281838],
            [1.25000, 0.0467735, 1.50000, 1.00000, 0.0660693, 0.0331131],
            [0.750000, 0.0549541, 0.500000, 1.00000, 0.0389045, 0.0776247],
            [1.25000, 0.0741310, 1.00000, 1.50000, 0.0524807, 0.104713],
            [1.75000, 0.0562341, 1.50000, 2.00000, 0.0398107, 0.0794328],
            [2.75000, 0.0794328, 2.00000, 3.50000, 0.0562341, 0.112202],
            [4.00000, 0.0309030, 3.50000, 4.50000, 0.0489779, 0.0194984],
            [0.250000, 0.0398107, 0.00000, 0.500000, 0.0239883, 0.0812831],
            [0.750000, 0.0446684, 0.500000, 1.00000, 0.0323594, 0.0776247],
            [1.25000, 0.0630957, 1.00000, 1.50000, 0.0478630, 0.109648],
            [1.75000, 0.0645654, 1.50000, 2.00000, 0.0489779, 0.112202],
            [2.50000, 0.0831764, 2.00000, 3.00000, 0.0512861, 0.158489],
            [3.50000, 0.0776247, 3.00000, 4.00000, 0.0416869, 0.169824],
            [4.50000, 0.0977237, 4.00000, 5.00000, 0.0416869, 0.269153],
            [5.50000, 0.0426580, 5.00000, 6.00000, 0.0177828, 0.165959],
            [3.00000, 0.120226, 2.00000, 4.00000, 0.173780, 0.0831764],
            [3.04000, 0.128825, 2.69000, 3.39000, 0.151356, 0.109648],
            [4.13000, 0.114815, 3.78000, 4.48000, 0.144544, 0.0912011],
            [0.350000, 0.0346737, 0.200000, 0.500000, 0.0537032, 0.0165959],
            [0.750000, 0.0512861, 0.500000, 1.00000, 0.0575440, 0.0436516],
            [1.50000, 0.0691831, 1.00000, 2.00000, 0.0758578, 0.0630957],
            [2.50000, 0.147911, 2.00000, 3.00000, 0.169824, 0.128825],
            [3.50000, 0.0645654, 3.00000, 4.00000, 0.0776247, 0.0512861],
        ],
        dtype=np.float32,
    )

    ObsRedshift = ObsSFRdensity[:, 0]
    xErrLo = np.abs(ObsSFRdensity[:, 0] - ObsSFRdensity[:, 2])
    xErrHi = np.abs(ObsSFRdensity[:, 3] - ObsSFRdensity[:, 0])

    ObsSFR = np.log10(ObsSFRdensity[:, 1])
    yErrLo = np.abs(np.log10(ObsSFRdensity[:, 1]) - np.log10(ObsSFRdensity[:, 4]))
    yErrHi = np.abs(np.log10(ObsSFRdensity[:, 5]) - np.log10(ObsSFRdensity[:, 1]))

    # Plot observational data
    ax.errorbar(
        ObsRedshift,
        ObsSFR,
        yerr=[yErrLo, yErrHi],
        xerr=[xErrLo, xErrHi],
        color="g",
        lw=1.0,
        alpha=0.3,
        marker="o",
        ls="none",
        label="Observations",
    )

    # Check if we have any snapshots
    if len(snapshots) == 0:
        print("No snapshot data available for SFR density evolution plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No snapshot data available for SFR density evolution plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"SFR_Density_Evolution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Calculate SFR density for each snapshot
    sfr_density = []
    redshifts = []

    for snap, (galaxies, volume, metadata) in snapshots.items():
        # Get the redshift for this snapshot
        redshift = metadata.get("redshift", 0.0)
        redshifts.append(redshift)

        # Extract hubble_h from metadata
        hubble_h = metadata.get("hubble_h", 0.73)

        # Skip if volume is zero (no valid data)
        if volume == 0:
            sfr_density.append(0.0)
            continue

        # Sum SFR and normalize by volume
        sfr_sum = np.sum(galaxies.SfrDisk + galaxies.SfrBulge)
        sfr_density.append(sfr_sum / volume * hubble_h**3)

    # Convert to numpy arrays
    redshifts = np.array(redshifts)
    sfr_density = np.array(sfr_density)

    # Sort by redshift
    sort_idx = np.argsort(redshifts)
    redshifts = redshifts[sort_idx]
    sfr_density = sfr_density[sort_idx]

    # Debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of snapshots: {len(snapshots)}")
        print(f"  Redshifts available: {redshifts}")
        print(f"  SFR density values: {sfr_density}")

    # Plot the model results
    nonzero = np.where(sfr_density > 0.0)[0]
    if len(nonzero) > 0:
        if verbose:
            print(f"  Plotting {len(nonzero)} nonzero SFR density points")
        # Use blue color to match the original plot
        ax.plot(
            redshifts[nonzero],
            np.log10(sfr_density[nonzero]),
            "b-",
            lw=3.0,
            label="Model",
        )
    else:
        # Always show warnings even without verbose
        print("  WARNING: No nonzero SFR density points to plot!")

    # Customize the plot
    ax.set_ylabel(get_sfr_density_label(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_redshift_label(), fontsize=AXIS_LABEL_SIZE)

    ax.xaxis.set_minor_locator(MultipleLocator(1))
    ax.yaxis.set_minor_locator(MultipleLocator(0.5))

    ax.set_xlim(0.0, 8.0)
    ax.set_ylim(-3.0, -0.4)

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

    output_path = os.path.join(output_dir, f"SFR_Density_Evolution{output_format}")
    if verbose:
        print(f"Saving SFR Density Evolution to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
