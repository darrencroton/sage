#!/usr/bin/env python

"""
SAGE Gas Mass Function Plot

This module generates a gas mass function plot from SAGE galaxy data.
"""

import os

import matplotlib.pyplot as plt
import numpy as np
from figures import (
    AXIS_LABEL_SIZE,
    IN_FIGURE_TEXT_SIZE,
    LEGEND_FONT_SIZE,
    get_gas_mass_label,
    get_mass_function_labels,
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
    Create a gas mass function plot.

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
    # Extract necessary metadata
    hubble_h = metadata["hubble_h"]

    # Set up the figure
    fig, ax = plt.subplots(figsize=(8, 6))

    # Apply consistent font settings
    setup_plot_fonts(ax)

    # Set up binning
    binwidth = 0.1  # mass function histogram bin width

    # Prepare data for cold gas
    w = np.where(galaxies.ColdGas > 0.0)[0]

    # Check if we have any galaxies to plot
    if len(w) == 0:
        print("No galaxies found with cold gas > 0.0")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No galaxies found with cold gas > 0.0",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"GasMassFunction{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    mass = np.log10(galaxies.ColdGas[w] * 1.0e10 / hubble_h)

    # Calculate specific SFR for red/blue division
    sfr = galaxies.SfrDisk[w] + galaxies.SfrBulge[w]
    stellar_mass = galaxies.StellarMass[w] * 1.0e10 / hubble_h

    # Avoid division by zero
    nonzero_mass = stellar_mass > 0
    ssfr = np.zeros_like(stellar_mass)
    if np.any(nonzero_mass):
        ssfr[nonzero_mass] = sfr[nonzero_mass] / stellar_mass[nonzero_mass]

    ssfr_cut = -11.0  # log10(sSFR) cut between red and blue galaxies

    # Set up histogram bins
    mi = np.floor(min(mass)) - 2
    ma = np.floor(max(mass)) + 2
    nbins = int((ma - mi) / binwidth)

    # Calculate histogram for all galaxies
    counts, binedges = np.histogram(mass, range=(mi, ma), bins=nbins)
    xaxis = binedges[:-1] + 0.5 * binwidth

    # Plot the main histogram
    ax.plot(
        xaxis, counts / volume * hubble_h**3 / binwidth, "k-", label="Model - Cold Gas"
    )

    # Add observational data from Zwaan et al. 2005 (HI)
    Zwaan = np.array(
        [
            [6.933, -0.333],
            [7.057, -0.490],
            [7.209, -0.698],
            [7.365, -0.667],
            [7.528, -0.823],
            [7.647, -0.958],
            [7.809, -0.917],
            [7.971, -0.948],
            [8.112, -0.927],
            [8.263, -0.917],
            [8.404, -1.062],
            [8.566, -1.177],
            [8.707, -1.177],
            [8.853, -1.312],
            [9.010, -1.344],
            [9.161, -1.448],
            [9.302, -1.604],
            [9.448, -1.792],
            [9.599, -2.021],
            [9.740, -2.406],
            [9.897, -2.615],
            [10.053, -3.031],
            [10.178, -3.677],
            [10.335, -4.448],
            [10.492, -5.083],
        ],
        dtype=np.float32,
    )

    # Add observational data from Obreschkow & Rawlings 2009 (H2)
    ObrRaw = np.array(
        [
            [7.300, -1.104],
            [7.576, -1.302],
            [7.847, -1.250],
            [8.133, -1.240],
            [8.409, -1.344],
            [8.691, -1.479],
            [8.956, -1.792],
            [9.231, -2.271],
            [9.507, -3.198],
            [9.788, -5.062],
        ],
        dtype=np.float32,
    )

    # Add observational data from Obreschkow & Rawlings 2009 (Cold Gas)
    ObrCold = np.array(
        [
            [8.009, -1.042],
            [8.215, -1.156],
            [8.409, -0.990],
            [8.604, -1.156],
            [8.799, -1.208],
            [9.020, -1.333],
            [9.194, -1.385],
            [9.404, -1.552],
            [9.599, -1.677],
            [9.788, -1.812],
            [9.999, -2.312],
            [10.172, -2.656],
            [10.362, -3.500],
            [10.551, -3.635],
            [10.740, -5.010],
        ],
        dtype=np.float32,
    )

    ObrCold_xval = np.log10(10 ** (ObrCold[:, 0]) / hubble_h / hubble_h)
    ObrCold_yval = 10 ** (ObrCold[:, 1]) * hubble_h**3
    Zwaan_xval = np.log10(10 ** (Zwaan[:, 0]) / hubble_h / hubble_h)
    Zwaan_yval = 10 ** (Zwaan[:, 1]) * hubble_h**3
    ObrRaw_xval = np.log10(10 ** (ObrRaw[:, 0]) / hubble_h / hubble_h)
    ObrRaw_yval = 10 ** (ObrRaw[:, 1]) * hubble_h**3

    ax.plot(
        ObrCold_xval,
        ObrCold_yval,
        color="black",
        lw=7,
        alpha=0.25,
        label="Obr. & Raw. 2009 (Cold Gas)",
    )
    ax.plot(
        Zwaan_xval,
        Zwaan_yval,
        color="cyan",
        lw=7,
        alpha=0.25,
        label="Zwaan et al. 2005 (HI)",
    )
    ax.plot(
        ObrRaw_xval,
        ObrRaw_yval,
        color="magenta",
        lw=7,
        alpha=0.25,
        label="Obr. & Raw. 2009 (H2)",
    )

    # Customize the plot
    ax.set_yscale("log")
    ax.set_xlim(8.0, 11.5)
    ax.set_ylim(1.0e-6, 1.0e-1)
    ax.xaxis.set_minor_locator(MultipleLocator(0.1))

    ax.set_ylabel(get_mass_function_labels(), fontsize=AXIS_LABEL_SIZE)
    ax.set_xlabel(get_gas_mass_label(), fontsize=AXIS_LABEL_SIZE)

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

    output_path = os.path.join(output_dir, f"GasMassFunction{output_format}")
    if verbose:
        print(f"Saving gas mass function to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
