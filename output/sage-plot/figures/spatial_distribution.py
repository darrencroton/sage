#!/usr/bin/env python

"""
SAGE Spatial Distribution Plot

This module generates a multi-panel plot showing the spatial distribution of galaxies.
"""

import os
import random

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
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
    Create a spatial distribution plot.

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

    # Create a figure with 3 subplots (arranged in a 2x2 grid, with one empty)
    fig, axes = plt.subplots(2, 2, figsize=(10, 10))
    axes = axes.flatten()

    # Hide the empty subplot (bottom right)
    axes[3].set_visible(False)

    # Apply consistent font settings to each subplot
    for ax in axes[:3]:
        setup_plot_fonts(ax)

    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]
    box_size = metadata.get("box_size", 62.5)  # Default to Mini-Millennium

    # Filter for galaxies with non-zero halo mass and significant stellar mass
    w = np.where((galaxies.Mvir > 0.0) & (galaxies.StellarMass > 0.1))[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with Mvir > 0 and StellarMass > 0.1")
        # Create an empty plot with a message
        for ax in axes[:3]:
            ax.text(
                0.5,
                0.5,
                "No galaxies found with Mvir > 0 and StellarMass > 0.1",
                horizontalalignment="center",
                verticalalignment="center",
                transform=ax.transAxes,
                fontsize=IN_FIGURE_TEXT_SIZE,
            )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"SpatialDistribution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # If we have too many galaxies, randomly sample a subset
    dilute = 10000  # Higher limit for this plot to show structure
    if len(w) > dilute:
        w = random.sample(list(w), dilute)

    # Get positions
    xx = galaxies.Pos[w, 0]
    yy = galaxies.Pos[w, 1]
    zz = galaxies.Pos[w, 2]

    # Add a small buffer around the box for the plot
    buffer = box_size * 0.1

    # Print some debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of galaxies plotted: {len(w)}")
        print(f"  Box size: {box_size}")
        print(f"  X position range: {min(xx):.2f} to {max(xx):.2f}")

    # Plot X-Y projection
    axes[0].scatter(xx, yy, marker="o", s=0.3, c="k", alpha=0.5)
    axes[0].set_xlabel(r"X (h$^{-1}$ Mpc)", fontsize=AXIS_LABEL_SIZE)
    axes[0].set_ylabel(r"Y (h$^{-1}$ Mpc)", fontsize=AXIS_LABEL_SIZE)
    axes[0].set_xlim(0.0 - buffer, box_size + buffer)
    axes[0].set_ylim(0.0 - buffer, box_size + buffer)

    # Plot X-Z projection
    axes[1].scatter(xx, zz, marker="o", s=0.3, c="k", alpha=0.5)
    axes[1].set_xlabel(r"X (h$^{-1}$ Mpc)", fontsize=AXIS_LABEL_SIZE)
    axes[1].set_ylabel(r"Z (h$^{-1}$ Mpc)", fontsize=AXIS_LABEL_SIZE)
    axes[1].set_xlim(0.0 - buffer, box_size + buffer)
    axes[1].set_ylim(0.0 - buffer, box_size + buffer)

    # Plot Y-Z projection
    axes[2].scatter(yy, zz, marker="o", s=0.3, c="k", alpha=0.5)
    axes[2].set_xlabel(r"Y (h$^{-1}$ Mpc)", fontsize=AXIS_LABEL_SIZE)
    axes[2].set_ylabel(r"Z (h$^{-1}$ Mpc)", fontsize=AXIS_LABEL_SIZE)
    axes[2].set_xlim(0.0 - buffer, box_size + buffer)
    axes[2].set_ylim(0.0 - buffer, box_size + buffer)

    # Adjust layout
    plt.tight_layout()

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"SpatialDistribution{output_format}")
    if verbose:
        print(f"Saving Spatial Distribution plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
