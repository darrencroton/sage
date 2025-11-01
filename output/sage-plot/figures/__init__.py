"""
SAGE Figure Modules

This package contains self-contained modules for creating various plots from SAGE galaxy data.
"""

# Standard figure settings for consistent appearance
"""Standard figure settings for consistent appearance across all plots."""
AXIS_LABEL_SIZE = 16  # Font size for axis labels
TICK_LABEL_SIZE = 12  # Font size for tick labels
LEGEND_FONT_SIZE = 12  # Size for legend text (use numeric size instead of 'large')
IN_FIGURE_TEXT_SIZE = 12  # Size for text inside figures (annotations, etc.)


def setup_plot_fonts(ax):
    """Apply consistent font sizes to a plot."""
    # Increase tick label sizes
    ax.tick_params(axis="both", which="major", labelsize=TICK_LABEL_SIZE)
    ax.tick_params(axis="both", which="minor", labelsize=TICK_LABEL_SIZE)

    # Configure global font sizes
    import matplotlib.pyplot as plt

    plt.rcParams.update(
        {
            "font.size": TICK_LABEL_SIZE,
            "legend.fontsize": LEGEND_FONT_SIZE,
            "figure.titlesize": AXIS_LABEL_SIZE,
        }
    )

    # Make sure all labels in legends will use the same font size
    import matplotlib as mpl

    mpl.rcParams["legend.fontsize"] = LEGEND_FONT_SIZE

    return ax


def setup_legend(ax, loc="best", frameon=False):
    """Create a consistently styled legend."""
    leg = ax.legend(loc=loc, numpoints=1, labelspacing=0.1, frameon=frameon)
    for t in leg.get_texts():
        t.set_fontsize(LEGEND_FONT_SIZE)
    return leg


# Utility functions for consistent LaTeX-free labels
def get_mass_function_labels():
    """Return consistent axis labels for mass function plots."""
    y_label = r"$\phi$ (Mpc$^{-3}$ dex$^{-1}$)"
    return y_label


# PHYSICS DISABLED: Halo-only utility functions
def get_halo_mass_label():
    """Return consistent x-axis label for halo mass plots."""
    x_label = r"log$_{10}$ M$_{\rm halo}$ (M$_{\odot}$)"
    return x_label


def get_spin_parameter_label():
    """Return consistent x-axis label for spin parameter plots."""
    x_label = r"Spin Parameter"
    return x_label


def get_redshift_label():
    """Return consistent x-axis label for redshift plots."""
    x_label = r"redshift"
    return x_label


def get_vmax_label():
    """Return consistent x-axis label for Vmax plots."""
    x_label = r"log$_{10}$ V$_{\rm max}$ (km/s)"
    return x_label


# PHYSICS DISABLED: Only halo-property plots remain
# Import all the figure modules so they can be discovered
from . import (
    halo_mass_function,
    halo_occupation,
    hmf_evolution,
    spatial_distribution,
    spin_distribution,
    velocity_distribution,
)

# Define available plot types
"""List of all available snapshot plot modules (halo properties only)."""
SNAPSHOT_PLOTS = [
    "halo_mass_function",
    "halo_occupation",
    "spin_distribution",
    "velocity_distribution",
    "spatial_distribution",
]

"""List of all available evolution plot modules (halo properties only)."""
EVOLUTION_PLOTS = [
    "hmf_evolution",
]

# Make sure this dictionary matches the classifications above

"""Mapping of plot names to their corresponding functions (halo properties only)."""
PLOT_FUNCS = {
    "halo_mass_function": halo_mass_function.plot,
    "halo_occupation": halo_occupation.plot,
    "hmf_evolution": hmf_evolution.plot,
    "spin_distribution": spin_distribution.plot,
    "velocity_distribution": velocity_distribution.plot,
    "spatial_distribution": spatial_distribution.plot,
}
