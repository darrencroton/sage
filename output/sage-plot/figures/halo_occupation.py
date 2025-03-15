#!/usr/bin/env python

"""
SAGE Halo Occupation Distribution Plot

This module generates a plot showing the occupation number of galaxies vs. halo mass.
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
    Create a halo occupation distribution plot.

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

    # Only use galaxies with non-zero Mvir
    valid_galaxies = np.where(galaxies.Mvir > 0.0)[0]

    # Check if we have any galaxies to plot
    if len(valid_galaxies) == 0:
        print("No galaxies found with Mvir > 0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with Mvir > 0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"HaloOccupation{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Group galaxies by distinct halos, using CentralGalaxyIndex
    halo_indices = np.unique(galaxies.CentralGalaxyIndex)

    # Initialize arrays for the results
    halo_mass = []  # Mass of each halo
    occupation_all = []  # Number of galaxies per halo
    occupation_central = []  # Number of central galaxies per halo (should be 1)
    occupation_satellite = []  # Number of satellite galaxies per halo

    # Process each halo
    for halo_idx in halo_indices:
        # Skip invalid indices
        if halo_idx < 0:
            continue

        # Find all galaxies in this halo
        halo_mask = galaxies.CentralGalaxyIndex == halo_idx
        halo_galaxies = galaxies[halo_mask]

        # Skip if no galaxies found
        if len(halo_galaxies) == 0:
            continue

        # Get the central galaxy (Type == 0)
        central_mask = halo_galaxies.Type == 0
        central_galaxies = halo_galaxies[central_mask]

        # Skip if no central galaxy found (shouldn't happen, but just in case)
        if len(central_galaxies) == 0:
            continue

        # Get the satellite galaxies (Type == 1)
        satellite_mask = halo_galaxies.Type == 1
        satellite_galaxies = halo_galaxies[satellite_mask]

        # Get the halo mass from the central galaxy
        # Convert to physical units (Msun)
        mvir = central_galaxies[0].Mvir * 1.0e10 / hubble_h

        # Store results
        halo_mass.append(mvir)
        occupation_all.append(len(halo_galaxies))
        occupation_central.append(len(central_galaxies))
        occupation_satellite.append(len(satellite_galaxies))

    # Convert to numpy arrays
    halo_mass = np.array(halo_mass)
    occupation_all = np.array(occupation_all)
    occupation_central = np.array(occupation_central)
    occupation_satellite = np.array(occupation_satellite)

    # Print some debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of halos: {len(halo_mass)}")
        print(
            f"  Halo mass range: {np.log10(min(halo_mass)):.2f} to {np.log10(max(halo_mass)):.2f}"
        )
        print(
            f"  Occupation number range: {min(occupation_all)} to {max(occupation_all)}"
        )

    # Bin the data by halo mass for the mean occupation
    bin_width = 0.2  # dex
    mass_bins = np.arange(10.0, 15.0, bin_width)
    mean_occupation_all = np.zeros(len(mass_bins) - 1)
    mean_occupation_central = np.zeros(len(mass_bins) - 1)
    mean_occupation_satellite = np.zeros(len(mass_bins) - 1)
    bin_counts = np.zeros(len(mass_bins) - 1)

    # Calculate mean occupation in each bin
    # Handle log10 of potentially zero values safely
    log_halo_mass = np.zeros_like(halo_mass)
    valid_mass = halo_mass > 0
    if np.any(valid_mass):
        log_halo_mass[valid_mass] = np.log10(halo_mass[valid_mass])

    for i in range(len(mass_bins) - 1):
        bin_mask = (log_halo_mass >= mass_bins[i]) & (log_halo_mass < mass_bins[i + 1])

        if np.sum(bin_mask) > 0:
            mean_occupation_all[i] = np.mean(occupation_all[bin_mask])
            mean_occupation_central[i] = np.mean(occupation_central[bin_mask])
            mean_occupation_satellite[i] = np.mean(occupation_satellite[bin_mask])
            bin_counts[i] = np.sum(bin_mask)

    # Plot the results
    bin_centers = mass_bins[:-1] + bin_width / 2

    # Only plot bins with data
    valid_bins = bin_counts > 0

    # Plot mean occupation for all galaxies
    ax.plot(
        bin_centers[valid_bins],
        mean_occupation_all[valid_bins],
        "k-",
        label="All galaxies",
    )

    # Plot mean occupation for central galaxies (should be close to 1)
    ax.plot(
        bin_centers[valid_bins],
        mean_occupation_central[valid_bins],
        "b--",
        label="Central galaxies",
    )

    # Plot mean occupation for satellite galaxies
    ax.plot(
        bin_centers[valid_bins],
        mean_occupation_satellite[valid_bins],
        "r-.",
        label="Satellite galaxies",
    )

    # Set log scale for y-axis
    ax.set_yscale("log")

    # Customize the plot
    ax.set_xlabel(r"log$_{10}$ M$_{\rm halo}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(
        r"Mean occupation number $\langle N \rangle$", fontsize=AXIS_LABEL_SIZE
    )

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))

    # Set axis limits
    ax.set_xlim(11.0, 15.0)
    ax.set_ylim(0.1, 100.0)

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

    output_path = os.path.join(output_dir, f"HaloOccupation{output_format}")
    if verbose:
        print(f"Saving Halo Occupation Distribution plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
