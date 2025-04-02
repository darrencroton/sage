#!/usr/bin/env python

"""
SAGE Spin Distribution Plot

This module generates a plot showing the distribution of galaxy spin parameters.
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
    Create a spin parameter distribution plot.

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

    # Filter for valid galaxies with non-zero Vvir and Rvir
    valid_galaxies = np.where(
        (galaxies.Vvir > 0.0)
        & (galaxies.Rvir > 0.0)
        & (
            galaxies.Spin[:, 0] ** 2
            + galaxies.Spin[:, 1] ** 2
            + galaxies.Spin[:, 2] ** 2
            > 0.0
        )
    )[0]

    # Check if we have any galaxies to plot
    if len(valid_galaxies) == 0:
        print("No valid galaxies found for spin distribution plot")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No valid galaxies found for spin distribution plot",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"SpinDistribution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    # Calculate spin parameter according to the formula:
    # λ = |J| / (√2 * Vvir * Rvir)
    # where |J| is the magnitude of the angular momentum vector (Spin in SAGE)
    spin_magnitude = np.sqrt(
        galaxies.Spin[valid_galaxies, 0] ** 2
        + galaxies.Spin[valid_galaxies, 1] ** 2
        + galaxies.Spin[valid_galaxies, 2] ** 2
    )

    spin_parameter = spin_magnitude / (
        np.sqrt(2) * galaxies.Vvir[valid_galaxies] * galaxies.Rvir[valid_galaxies]
    )

    # Filter out any invalid values
    valid_spins = np.where(
        (spin_parameter > 0.0) & (spin_parameter < 1.0) & np.isfinite(spin_parameter)
    )[0]

    if len(valid_spins) == 0:
        print("No valid spin parameter values found")
        # Create an empty plot with a message
        ax.text(
            0.5,
            0.5,
            "No valid spin parameter values found",
            horizontalalignment="center",
            verticalalignment="center",
            transform=ax.transAxes,
            fontsize=IN_FIGURE_TEXT_SIZE,
        )

        # Save the figure
        os.makedirs(output_dir, exist_ok=True)
        output_path = os.path.join(output_dir, f"SpinDistribution{output_format}")
        plt.savefig(output_path)
        plt.close()
        return output_path

    spin_parameter = spin_parameter[valid_spins]

    # Print some debug information
    # Print some debug information if verbose mode is enabled
    if verbose:
        print(f"  Number of galaxies with valid spin: {len(spin_parameter)}")
        print(
            f"  Spin parameter range: {min(spin_parameter):.4f} to {max(spin_parameter):.4f}"
        )
        print(f"  Mean spin parameter: {np.mean(spin_parameter):.4f}")
        print(f"  Median spin parameter: {np.median(spin_parameter):.4f}")

    # Create histogram of spin parameters
    bin_min = -0.02
    bin_max = 0.5
    bin_width = 0.01
    bins = int((bin_max - bin_min) / bin_width)

    counts, bin_edges = np.histogram(
        spin_parameter, range=(bin_min, bin_max), bins=bins
    )
    bin_centers = bin_edges[:-1] + bin_width / 2

    # Plot the histogram
    ax.plot(bin_centers, counts, "k-", lw=2, label="Simulation")

    # Add a theoretical log-normal distribution for comparison (optional)
    # Parameters for log-normal are typical for dark matter halos
    lambda_0 = 0.035  # Typical value for dark matter halos
    sigma = 0.5  # Typical value for width of the distribution

    # Create log-normal distribution (normalized to match the histogram peak)
    x = np.linspace(0.001, 0.5, 1000)
    p_lognormal = (1 / (x * sigma * np.sqrt(2 * np.pi))) * np.exp(
        -np.log(x / lambda_0) ** 2 / (2 * sigma**2)
    )

    # Normalize to match the histogram peak
    norm_factor = max(counts) / max(p_lognormal)
    p_lognormal *= norm_factor

    # Plot theoretical distribution
    ax.plot(x, p_lognormal, "r--", lw=1.5, label=r"Log-normal ($\lambda_0=0.035$)")

    # Customize the plot
    ax.set_xlabel(r"Spin Parameter", fontsize=AXIS_LABEL_SIZE)
    ax.set_ylabel(r"Number", fontsize=AXIS_LABEL_SIZE)

    # Set the x and y axis minor ticks with MaxNLocator to avoid excessive ticks
    ax.xaxis.set_minor_locator(MultipleLocator(0.01))
    # Use MaxNLocator instead to prevent too many ticks
    ax.yaxis.set_minor_locator(MaxNLocator(10))

    # Set axis limits
    ax.set_xlim(0.0, 0.25)
    y_max = max(counts) * 1.1
    ax.set_ylim(0, y_max)

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

    output_path = os.path.join(output_dir, f"SpinDistribution{output_format}")
    if verbose:
        print(f"Saving Spin Distribution plot to: {output_path}")
    plt.savefig(output_path)
    plt.close()

    return output_path
