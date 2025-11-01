#!/usr/bin/env python

"""
SAGE Quiescent Fraction Plot

This module generates a plot showing the fraction of quiescent galaxies as a function of stellar mass.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
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
    Create a quiescent fraction vs. stellar mass plot.

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

    # Define the sSFR cutoff for quiescent galaxies (same as in original code)
    sSFRcut = -11.0  # Log10 of specific SFR in yr^-1

    # Define the stellar mass threshold for "group" scale halos (log10 Msun)
    groupscale = 12.5

    # Filter for galaxies with non-zero stellar mass
    w = np.where(galaxies.StellarMass > 0.0)[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with stellar mass > 0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with stellar mass > 0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"QuiescentFraction{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Calculate required quantities for all selected galaxies - safely handle zeros
    StellarMass = np.log10(galaxies.StellarMass[w] * 1.0e10 / hubble_h)

    # Safely handle CentralMvir values (avoid log10 of zero/negative)
    valid_mvir = galaxies.CentralMvir[w] > 0
    CentralMvir = np.full(len(w), np.nan)  # Initialize with NaN
    CentralMvir[valid_mvir] = np.log10(
        galaxies.CentralMvir[w][valid_mvir] * 1.0e10 / hubble_h
    )

    Type = galaxies.Type[w]

    # Safely calculate sSFR (avoid division by zero)
    sSFR = np.zeros(len(w))
    stellar_mass_phys = galaxies.StellarMass[w] * 1.0e10 / hubble_h
    nonzero_mass = stellar_mass_phys > 0

    if np.any(nonzero_mass):
        sfr = galaxies.SfrDisk[w][nonzero_mass] + galaxies.SfrBulge[w][nonzero_mass]
        sSFR[nonzero_mass] = sfr / stellar_mass_phys[nonzero_mass]

    # Define mass bins
    min_range = 9.5
    max_range = 12.0
    interval = 0.1
    nbins = int((max_range - min_range) / interval)
    mass_bins = np.arange(min_range, max_range, interval)

    # Arrays to store results
    mass = []  # Bin centers
    fraction = []  # All galaxies
    central_fraction = []  # Central galaxies
    satellite_fraction = []  # Satellite galaxies
    satellite_fraction_lo = []  # Satellites in low-mass halos
    satellite_fraction_hi = []  # Satellites in high-mass halos

    # Calculate fractions for each mass bin
    for i in range(nbins - 1):
        # All galaxies in this mass bin
        this_bin_mask = (StellarMass >= mass_bins[i]) & (StellarMass < mass_bins[i + 1])
        this_bin_count = np.sum(this_bin_mask)

        if this_bin_count > 0:
            # Quiescent galaxies in this mass bin
            quiescent_mask = this_bin_mask & (sSFR < 10.0**sSFRcut)
            quiescent_count = np.sum(quiescent_mask)

            # Calculate fraction
            fraction.append(quiescent_count / this_bin_count)
        else:
            fraction.append(0.0)

        # Central galaxies in this mass bin
        central_bin_mask = this_bin_mask & (Type == 0)
        central_bin_count = np.sum(central_bin_mask)

        if central_bin_count > 0:
            # Quiescent central galaxies
            quiescent_central_mask = central_bin_mask & (sSFR < 10.0**sSFRcut)
            quiescent_central_count = np.sum(quiescent_central_mask)

            # Calculate fraction
            central_fraction.append(quiescent_central_count / central_bin_count)
        else:
            central_fraction.append(0.0)

        # Satellite galaxies in this mass bin
        satellite_bin_mask = this_bin_mask & (Type == 1)
        satellite_bin_count = np.sum(satellite_bin_mask)

        if satellite_bin_count > 0:
            # Quiescent satellite galaxies
            quiescent_satellite_mask = satellite_bin_mask & (sSFR < 10.0**sSFRcut)
            quiescent_satellite_count = np.sum(quiescent_satellite_mask)

            # Calculate fraction
            satellite_fraction.append(quiescent_satellite_count / satellite_bin_count)

            # For satellites, the original code used the count of ALL satellites as denominator
            # for both the satellite fraction and the lo/hi satellite fractions
            
            # Low-mass halo satellites
            low_mass_mask = satellite_bin_mask & (CentralMvir < groupscale)
            quiescent_low_mass_mask = low_mass_mask & (sSFR < 10.0**sSFRcut)
            quiescent_low_mass_count = np.sum(quiescent_low_mass_mask)
            
            # We use satellite_bin_count as the denominator (all satellites in this mass bin)
            # to ensure low + high fractions add up to the total satellite fraction
            satellite_fraction_lo.append(quiescent_low_mass_count / satellite_bin_count if satellite_bin_count > 0 else 0.0)

            # High-mass halo satellites
            high_mass_mask = satellite_bin_mask & (CentralMvir >= groupscale)
            quiescent_high_mass_mask = high_mass_mask & (sSFR < 10.0**sSFRcut)
            quiescent_high_mass_count = np.sum(quiescent_high_mass_mask)
            
            # Same denominator as above for consistency
            satellite_fraction_hi.append(quiescent_high_mass_count / satellite_bin_count if satellite_bin_count > 0 else 0.0)
        else:
            satellite_fraction.append(0.0)
            satellite_fraction_lo.append(0.0)
            satellite_fraction_hi.append(0.0)

        # Store the mass bin center
        mass.append((mass_bins[i] + mass_bins[i + 1]) / 2.0)

    # Convert to numpy arrays
    mass = np.array(mass)
    fraction = np.array(fraction)
    central_fraction = np.array(central_fraction)
    satellite_fraction = np.array(satellite_fraction)
    satellite_fraction_lo = np.array(satellite_fraction_lo)
    satellite_fraction_hi = np.array(satellite_fraction_hi)

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Quiescent Fraction plot debug:")
        print(f"  Total number of galaxies: {len(w)}")
        print(f"  Number of mass bins: {nbins}")
        print(f"  Stellar mass range: {min(StellarMass):.2f} to {max(StellarMass):.2f}")

    # Plot the fractions
    # All galaxies
    mask = fraction > 0
    if np.any(mask):
        ax.plot(mass[mask], fraction[mask], "k-", label="All")

    # Central galaxies
    mask = central_fraction > 0
    if np.any(mask):
        ax.plot(mass[mask], central_fraction[mask], "b-", label="Centrals")

    # Satellite galaxies
    mask = satellite_fraction > 0
    if np.any(mask):
        ax.plot(mass[mask], satellite_fraction[mask], "r-", label="Satellites")

    # Satellites in low-mass halos
    mask = satellite_fraction_lo > 0
    if np.any(mask):
        ax.plot(mass[mask], satellite_fraction_lo[mask], "r--", label="Satellites-Lo", linewidth=0.8)

    # Satellites in high-mass halos
    mask = satellite_fraction_hi > 0
    if np.any(mask):
        ax.plot(mass[mask], satellite_fraction_hi[mask], "r-", label="Satellites-Hi", linewidth=0.8, dashes=[8, 3])

    # Customize the plot
    ax.set_xlabel(get_stellar_mass_label(), fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"Quiescent Fraction", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))
    ax.yaxis.set_minor_locator(MultipleLocator(0.05))

    # Set axis limits - matching the original plot
    ax.set_xlim(9.5, 12.0)
    ax.set_ylim(0.0, 1.05)

    # Add consistently styled legend
    setup_legend(ax, loc="lower right")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"QuiescentFraction{output_format}")
    if verbose:
        print(f"Saving Quiescent Fraction plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
