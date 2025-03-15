#!/usr/bin/env python

"""
SAGE Velocity Distribution Plot

This module generates a plot showing the distribution of galaxy velocities.
"""

import os

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
    Create a velocity distribution plot.

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
    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]

    # Set up histogram binning
    bin_min = -40.0
    bin_max = 40.0
    bin_width = 0.5
    nbins = int((bin_max - bin_min) / bin_width)

    # Get position and velocity data
    pos_x = galaxies.Pos[:, 0] / hubble_h  # Convert to Mpc
    pos_y = galaxies.Pos[:, 1] / hubble_h
    pos_z = galaxies.Pos[:, 2] / hubble_h

    vel_x = galaxies.Vel[:, 0]  # km/s
    vel_y = galaxies.Vel[:, 1]
    vel_z = galaxies.Vel[:, 2]

    # Calculate line-of-sight distance and velocity
    # For line-of-sight, we use the position vector from the origin
    dist_los = np.sqrt(pos_x**2 + pos_y**2 + pos_z**2)

    # Skip galaxies with zero distance (to avoid division by zero)
    valid_galaxies = dist_los > 0.0

    # If no valid galaxies, create an empty plot
    if not np.any(valid_galaxies):
        print("No galaxies found with valid positions")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with valid positions",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"VelocityDistribution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Get line-of-sight velocity: v·r/|r| (projection of velocity onto position)
    pos_x = pos_x[valid_galaxies]
    pos_y = pos_y[valid_galaxies]
    pos_z = pos_z[valid_galaxies]
    vel_x = vel_x[valid_galaxies]
    vel_y = vel_y[valid_galaxies]
    vel_z = vel_z[valid_galaxies]
    dist_los = dist_los[valid_galaxies]

    # Line-of-sight velocity
    vel_los = (pos_x * vel_x + pos_y * vel_y + pos_z * vel_z) / dist_los

    # Distance including redshift: r + v/(H*100)
    # (standard approach for mock catalogs)
    dist_redshift = dist_los + vel_los / (hubble_h * 100.0)

    # Total number of galaxies for normalizing
    tot_gals = len(pos_x)

    # Print some debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of galaxies: {tot_gals}")
        print(
            f"  Line-of-sight velocity range: {min(vel_los):.2f} to {max(vel_los):.2f} km/s"
        )
        print(f"  X velocity range: {min(vel_x):.2f} to {max(vel_x):.2f} km/s")

    # Create histograms for each velocity component
    # Line-of-sight velocity
    counts_los, binedges = np.histogram(
        vel_los / (hubble_h * 100.0), range=(bin_min, bin_max), bins=nbins
    )
    bin_centers = binedges[:-1] + bin_width / 2
    ax.plot(
        bin_centers,
        counts_los / bin_width / tot_gals,
        "k-",
        lw=2,
        label="line-of-sight",
    )

    # X velocity component
    counts_x, _ = np.histogram(
        vel_x / (hubble_h * 100.0), range=(bin_min, bin_max), bins=nbins
    )
    ax.plot(
        bin_centers, counts_x / bin_width / tot_gals, "r-", lw=1.5, label="x-velocity"
    )

    # Y velocity component
    counts_y, _ = np.histogram(
        vel_y / (hubble_h * 100.0), range=(bin_min, bin_max), bins=nbins
    )
    ax.plot(
        bin_centers, counts_y / bin_width / tot_gals, "g-", lw=1.5, label="y-velocity"
    )

    # Z velocity component
    counts_z, _ = np.histogram(
        vel_z / (hubble_h * 100.0), range=(bin_min, bin_max), bins=nbins
    )
    ax.plot(
        bin_centers, counts_z / bin_width / tot_gals, "b-", lw=1.5, label="z-velocity"
    )

    # Use log scale for y-axis
    ax.set_yscale("log")

    # Customize the plot
    ax.set_xlabel(r"Velocity / H$_0$", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"Box Normalised Count", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(5))

    # Set axis limits
    ax.set_xlim(bin_min, bin_max)
    ax.set_ylim(1e-5, 0.5)

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

    output_path = os.path.join(output_dir, f"VelocityDistribution{output_format}")
    if verbose:
        print(f"Saving Velocity Distribution plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
