#!/usr/bin/env python

"""
SAGE Baryon Fraction Plot

This module generates a plot showing the baryon fraction vs. halo mass.
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
    Create a baryon fraction vs. halo mass plot.

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

    # Get the baryon fraction parameter (or use default cosmic value if not available)
    baryon_frac = params.get("BaryonFrac", 0.17) if params else 0.17

    # Only use central galaxies (Type = 0) with non-zero Mvir
    central_mask = (galaxies.Type == 0) & (galaxies.Mvir > 0.0)

    # Check if we have any central galaxies to plot
    if not np.any(central_mask):
        print("No central galaxies found with Mvir > 0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No central galaxies found with Mvir > 0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"BaryonFraction{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Set up halo mass bins
    min_halo = 11.0
    max_halo = 16.0
    interval = 0.1
    nbins = int((max_halo - min_halo) / interval)
    halo_bins = np.arange(min_halo, max_halo, interval)

    # Arrays to store results
    central_halo_mass = []  # Central halo mass
    mean_baryon_fraction = []  # Total baryon fraction
    mean_stars = []  # Stellar component
    mean_cold = []  # Cold gas component
    mean_hot = []  # Hot gas component
    mean_ejected = []  # Ejected gas component
    mean_ics = []  # Intracluster stars component
    mean_bh = []  # Black hole component

    # Loop through halo mass bins
    for i in range(nbins - 1):
        # Get central galaxies in this mass bin
        # Handle log10 of halo mass safely
        halo_mass = np.full(len(galaxies), -np.inf)  # Initialize with negative infinity
        valid_mvir = (galaxies.Mvir > 0) & central_mask
        halo_mass[valid_mvir] = np.log10(galaxies.Mvir[valid_mvir] * 1.0e10 / hubble_h)

        bin_mask = (
            central_mask & (halo_mass >= halo_bins[i]) & (halo_mass < halo_bins[i + 1])
        )
        centrals_in_bin = np.where(bin_mask)[0]

        # Skip if no central galaxies in this bin
        if (
            len(centrals_in_bin) < 3
        ):  # Require at least 3 galaxies for meaningful statistics
            continue

        baryon_fractions = []
        stellar_fractions = []
        cold_fractions = []
        hot_fractions = []
        ejected_fractions = []
        ics_fractions = []
        bh_fractions = []
        halo_masses = []

        # Calculate fractions for each central galaxy
        for j in centrals_in_bin:
            # Get all galaxies in this halo (central and satellites)
            halo_galaxies = np.where(
                galaxies.CentralGalaxyIndex == galaxies.CentralGalaxyIndex[j]
            )[0]

            if len(halo_galaxies) > 0:
                # Sum baryonic components across all galaxies in the halo
                stars = np.sum(galaxies.StellarMass[halo_galaxies])
                cold = np.sum(galaxies.ColdGas[halo_galaxies])
                hot = np.sum(galaxies.HotGas[halo_galaxies])
                ejected = np.sum(galaxies.EjectedMass[halo_galaxies])
                ics = np.sum(galaxies.IntraClusterStars[halo_galaxies])
                bh = np.sum(galaxies.BlackHoleMass[halo_galaxies])

                # Total baryons
                baryons = stars + cold + hot + ejected + ics + bh

                # Calculate fractions relative to halo mass
                baryon_fractions.append(baryons / galaxies.Mvir[j])
                stellar_fractions.append(stars / galaxies.Mvir[j])
                cold_fractions.append(cold / galaxies.Mvir[j])
                hot_fractions.append(hot / galaxies.Mvir[j])
                ejected_fractions.append(ejected / galaxies.Mvir[j])
                ics_fractions.append(ics / galaxies.Mvir[j])
                bh_fractions.append(bh / galaxies.Mvir[j])

                # Store the central halo mass (log10, in Msun)
                halo_masses.append(np.log10(galaxies.Mvir[j] * 1.0e10 / hubble_h))

        # Calculate means for this bin
        if len(baryon_fractions) > 0:
            central_halo_mass.append(np.mean(halo_masses))
            mean_baryon_fraction.append(np.mean(baryon_fractions))
            mean_stars.append(np.mean(stellar_fractions))
            mean_cold.append(np.mean(cold_fractions))
            mean_hot.append(np.mean(hot_fractions))
            mean_ejected.append(np.mean(ejected_fractions))
            mean_ics.append(np.mean(ics_fractions))
            mean_bh.append(np.mean(bh_fractions))

    # Convert to numpy arrays
    central_halo_mass = np.array(central_halo_mass)
    mean_baryon_fraction = np.array(mean_baryon_fraction)
    mean_stars = np.array(mean_stars)
    mean_cold = np.array(mean_cold)
    mean_hot = np.array(mean_hot)
    mean_ejected = np.array(mean_ejected)
    mean_ics = np.array(mean_ics)
    mean_bh = np.array(mean_bh)

    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"Baryon Fraction plot debug:")
        print(f"  Number of mass bins with data: {len(central_halo_mass)}")
        print(
            f"  Halo mass range: {min(central_halo_mass):.2f} to {max(central_halo_mass):.2f}"
        )
        print(
            f"  Mean baryon fraction range: {min(mean_baryon_fraction):.3f} to {max(mean_baryon_fraction):.3f}"
        )
        print(f"  Cosmic baryon fraction (parameter): {baryon_frac:.3f}")

    # Plot the results
    # Total baryon fraction
    ax.plot(central_halo_mass, mean_baryon_fraction, "k-", lw=2, label="TOTAL")

    # Fill between to show variance
    variance = np.zeros_like(mean_baryon_fraction) + 0.02  # Simple approximation
    ax.fill_between(
        central_halo_mass,
        np.clip(mean_baryon_fraction - variance, 0, None),
        np.clip(mean_baryon_fraction + variance, 0, None),
        facecolor="purple",
        alpha=0.25,
        label="TOTAL",
    )

    # Individual components
    ax.plot(central_halo_mass, mean_stars, "k--", label="Stars")
    ax.plot(central_halo_mass, mean_cold, "b-", label="Cold")
    ax.plot(central_halo_mass, mean_hot, "r-", label="Hot")
    ax.plot(central_halo_mass, mean_ejected, "g-", label="Ejected")
    ax.plot(central_halo_mass, mean_ics, "y-", label="ICS")

    # Black hole mass is typically too small to show up well
    # ax.plot(central_halo_mass, mean_bh, 'k:', label='BH')

    # Add a horizontal line showing the cosmic baryon fraction
    ax.axhline(
        y=baryon_frac,
        color="k",
        linestyle=":",
        lw=1.5,
        label=f"Cosmic: {baryon_frac:.2f}",
    )

    # Customize the plot
    ax.set_xlabel(
        r"Central log$_{10}$ M$_{\rm vir}$ (M$_{\odot}$)", fontsize=AXIS_LABEL_SIZE
    )
    ax.set_ylabel(r"Baryon Fraction", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.5))
    ax.yaxis.set_minor_locator(MultipleLocator(0.01))

    # Set axis limits - matching the original plot
    ax.set_xlim(10.8, 15.0)
    ax.set_ylim(0.0, max(0.23, max(mean_baryon_fraction) * 1.1))

    # Add consistently styled legend
    leg = setup_legend(ax, loc="upper right")

    # Save the figure, ensuring the output directory exists
    try:
        os.makedirs(output_dir, exist_ok=True)
    except Exception as e:
        print(f"Warning: Could not create output directory {output_dir}: {e}")
        # Try to use a subdirectory of the current directory as fallback
        output_dir = "./plots"
        os.makedirs(output_dir, exist_ok=True)

    output_path = os.path.join(output_dir, f"BaryonFraction{output_format}")
    if verbose:
        print(f"Saving Baryon Fraction plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
