#!/usr/bin/env python

"""
SAGE Halo Mass Function Evolution Plot

This module generates a halo mass function evolution plot from SAGE halo data.
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


def plot(snapshots, params, output_dir="plots", output_format=".png", verbose=False):
    """
    Create a halo mass function evolution plot.

    Args:
        snapshots: Dictionary mapping snapshot numbers to tuples of (galaxies, volume, metadata)
        params: Dictionary with SAGE parameters
        output_dir: Output directory for the plot
        output_format: File format for the output

    Returns:
        Path to the saved plot file
    """
    # Define target redshifts and their tolerances
    target_redshifts = [0.0, 1.0, 2.0, 3.0]
    target_tolerance = [0.2, 0.5, 0.5, 0.5]

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Set up binning
    binwidth = 0.1

    # Debug information
    if verbose:
        print(f"  Number of snapshots: {len(snapshots)}")
        for snap, (galaxies, volume, metadata) in snapshots.items():
            print(
                f"  Snapshot {snap}: {len(galaxies)} halos, z={metadata.get('redshift', 'unknown')}"
            )

    # Sort snapshots by redshift and filter to target redshifts
    sorted_snapshots = [
        (snap, galaxies, volume, metadata)
        for snap, (galaxies, volume, metadata) in snapshots.items()
    ]
    # Sort by redshift
    sorted_snapshots.sort(key=lambda x: x[3]["redshift"])

    # Filter snapshots to match target redshifts
    target_snapshots = []

    # For each target redshift, find the closest snapshot within tolerance
    for i, target_z in enumerate(target_redshifts):
        tolerance = target_tolerance[i]
        # Filter snapshots with z >= target_z
        candidates = [s for s in sorted_snapshots if s[3]["redshift"] >= target_z]
        if candidates:
            # Find the closest one
            closest = min(candidates, key=lambda x: abs(x[3]["redshift"] - target_z))
            # Check if it's within tolerance
            if abs(closest[3]["redshift"] - target_z) <= tolerance:
                target_snapshots.append(closest)
                if verbose:
                    print(f"Target z={target_z:.1f}: Using snapshot with z={closest[3]['redshift']:.3f}")
        elif verbose:
            print(f"Target z={target_z:.1f}: No suitable snapshot found")

    # Colors for different redshifts
    colors = ["k", "b", "g", "r", "m", "y", "c", "orange"]

    # Check if we have any snapshots to plot
    if len(target_snapshots) == 0:
        print("No snapshot data available for HMF evolution plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No snapshot data available for HMF evolution plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(
            output_dir, f"HaloMassFunction_Evolution{output_format}"
        )
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Plot model HMFs at target redshifts
    for i, (snap, galaxies, volume, metadata) in enumerate(target_snapshots):
        hubble_h = metadata["hubble_h"]
        redshift = metadata["redshift"]
        color = colors[i % len(colors)]

        # Debug output - only show if verbose is enabled
        if verbose:
            print(f"Processing HMF for snapshot {snap}, z={redshift:.1f}")

        # Select halos (Type 0 = central galaxies = halos) with valid mass
        w = np.where((galaxies.Type == 0) & (galaxies.Mvir > 0.0))[0]

        # Skip this snapshot if no halos found
        if len(w) == 0:
            print(f"No halos found for z={redshift:.1f}")
            continue

        mass = np.log10(galaxies.Mvir[w] * 1.0e10 / hubble_h)

        # Set up histogram bins
        mi = np.floor(min(mass)) - 1
        ma = np.floor(max(mass)) + 1

        # Force reasonable limits
        mi = max(mi, 10.0)
        ma = min(ma, 16.0)

        nbins = int((ma - mi) / binwidth)

        # Calculate histogram
        counts, binedges = np.histogram(mass, range=(mi, ma), bins=nbins)
        xaxis = binedges[:-1] + 0.5 * binwidth

        # Plot the histogram
        ax.plot(
            xaxis, counts / volume * hubble_h**3 / binwidth, color=color, linestyle="-", lw=2
        )

        # Store redshift values for labels in the top right corner
        if i == 0:
            # Initialize list of redshifts for the legend
            redshift_labels = []
        # Add this redshift to the list
        redshift_labels.append((redshift, color))

    # Customize the plot
    ax.set_yscale("log")
    ax.set_xlim(10.0, 15.5)
    ax.set_ylim(1.0e-6, 1.0e-1)
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))

    ax.set_ylabel(get_mass_function_labels(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_halo_mass_label(), fontsize=AXIS_LABEL_SIZE)

    # Add redshift labels in the top right corner
    if 'redshift_labels' in locals():
        # Sort labels by redshift
        redshift_labels.sort(key=lambda x: x[0])
        # Position for the first label
        x_pos = 15.3
        y_pos = 6e-2  # Near the top of the plot (log scale)
        for z, color in redshift_labels:
            ax.text(
                x_pos,
                y_pos,
                f"z = {z:.1f}",
                color=color,
                fontsize=IN_FIGURE_TEXT_SIZE,
                ha='right',
                va='top'
            )
            # Move down for the next label
            y_pos *= 0.6  # Reduces y-position by 40% each time (works well with log scale)

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(
        output_dir, f"HaloMassFunction_Evolution{output_format}"
    )
    if verbose:
        print(f"Saving Halo Mass Function Evolution to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
