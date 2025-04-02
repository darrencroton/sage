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
from matplotlib.ticker import MultipleLocator, MaxNLocator


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

    # Get only valid halo indices
    halo_indices = np.unique(galaxies.CentralGalaxyIndex)
    valid_indices = halo_indices[halo_indices >= 0]
    
    if verbose:
        print(f"  Processing {len(valid_indices)} unique halos")
    
    # Pre-allocate arrays for the results with known size
    halo_mass = np.zeros(len(valid_indices))
    occupation_all = np.zeros(len(valid_indices), dtype=np.int32)
    occupation_central = np.zeros(len(valid_indices), dtype=np.int32)
    occupation_satellite = np.zeros(len(valid_indices), dtype=np.int32)
    
    # Create a mapping from central galaxy index to position in our arrays
    # This avoids looping over individual halos and makes vectorized operations possible
    central_index_to_pos = {}
    for i, idx in enumerate(valid_indices):
        central_index_to_pos[idx] = i
    
    # Count all galaxies belonging to each halo in one vectorized operation
    unique_indices, counts = np.unique(galaxies.CentralGalaxyIndex, return_counts=True)
    
    # Filter out invalid indices and update occupation counts
    for i, idx in enumerate(unique_indices):
        if idx >= 0 and idx in central_index_to_pos:
            pos = central_index_to_pos[idx]
            occupation_all[pos] = counts[i]
    
    # Count central galaxies (Type == 0) and get their halo masses
    central_mask = (galaxies.Type == 0) & np.isin(galaxies.CentralGalaxyIndex, valid_indices)
    central_galaxies = galaxies[central_mask]
    
    for i, gal in enumerate(central_galaxies):
        if gal.CentralGalaxyIndex in central_index_to_pos:
            pos = central_index_to_pos[gal.CentralGalaxyIndex]
            occupation_central[pos] = 1  # Should be only one central per halo
            halo_mass[pos] = gal.Mvir * 1.0e10 / hubble_h  # Convert to physical units (Msun)
    
    # Compute satellite counts (occupation_all - occupation_central)
    occupation_satellite = occupation_all - occupation_central
    
    # Remove any remaining zeros in halo_mass (halos without centrals)
    valid_halos = halo_mass > 0
    halo_mass = halo_mass[valid_halos]
    occupation_all = occupation_all[valid_halos]
    occupation_central = occupation_central[valid_halos]
    occupation_satellite = occupation_satellite[valid_halos]

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

    # Calculate mean occupation in each bin using numpy's digitize for efficient binning
    log_halo_mass = np.log10(halo_mass)
    bin_indices = np.digitize(log_halo_mass, mass_bins)
    
    # Calculate means for each bin in a vectorized way
    for i in range(1, len(mass_bins)):
        bin_mask = bin_indices == i
        count = np.sum(bin_mask)
        if count > 0:
            mean_occupation_all[i-1] = np.mean(occupation_all[bin_mask])
            mean_occupation_central[i-1] = np.mean(occupation_central[bin_mask])
            mean_occupation_satellite[i-1] = np.mean(occupation_satellite[bin_mask])
            bin_counts[i-1] = count

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

    # Set the x and y axis minor ticks with MaxNLocator to avoid excessive ticks
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
