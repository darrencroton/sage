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


def get_stellar_mass_label():
    """Return consistent x-axis label for stellar mass plots."""
    x_label = r"log$_{10}$ M$_{\rm stars}$ (M$_{\odot}$)"
    return x_label


def get_baryonic_mass_label():
    """Return consistent x-axis label for baryonic mass plots."""
    x_label = r"log$_{10}$ M$_{\rm bar}$ (M$_{\odot}$)"
    return x_label


def get_gas_mass_label():
    """Return consistent x-axis label for gas mass plots."""
    x_label = r"log$_{10}$ M$_{\rm X}$ (M$_{\odot}$)"
    return x_label


def get_redshift_label():
    """Return consistent x-axis label for redshift plots."""
    x_label = r"redshift"
    return x_label


def get_sfr_density_label():
    """Return consistent y-axis label for SFR density plots."""
    y_label = r"log$_{10}$ SFR density (M$_{\odot}$ yr$^{-1}$ Mpc$^{-3}$)"
    return y_label


def get_ssfr_label():
    """Return consistent y-axis label for specific SFR plots."""
    y_label = r"log$_{10}$ sSFR (yr$^{-1}$)"
    return y_label


def get_vmax_label():
    """Return consistent x-axis label for Vmax plots."""
    x_label = r"log$_{10}$ V$_{\rm max}$ (km/s)"
    return x_label


def get_black_hole_mass_label():
    """Return consistent x-axis label for black hole mass plots."""
    x_label = r"log$_{10}$ M$_{\rm BH}$ (M$_{\odot}$)"
    return x_label


def get_bulge_mass_label():
    """Return consistent x-axis label for bulge mass plots."""
    x_label = r"log$_{10}$ M$_{\rm bulge}$ (M$_{\odot}$)"
    return x_label


def get_halo_mass_label():
    """Return consistent x-axis label for halo mass plots."""
    x_label = r"log$_{10}$ M$_{\rm halo}$ (M$_{\odot}$)"
    return x_label


def get_spin_parameter_label():
    """Return consistent x-axis label for spin parameter plots."""
    x_label = r"Spin Parameter"
    return x_label


# Import all the figure modules so they can be discovered
from . import (
    baryon_fraction,
    baryonic_mass_function,
    baryonic_tully_fisher,
    black_hole_bulge_relation,
    bulge_mass_fraction,
    gas_fraction,
    gas_mass_function,
    halo_occupation,
    mass_reservoir_scatter,
    metallicity,
    quiescent_fraction,
    sfr_density_evolution,
    smf_evolution,
    spatial_distribution,
    specific_sfr,
    spin_distribution,
    stellar_mass_density_evolution,
    stellar_mass_function,
    velocity_distribution,
)

# Define available plot types
"""List of all available snapshot plot modules."""
SNAPSHOT_PLOTS = [
    "stellar_mass_function",
    "baryonic_mass_function",
    "gas_mass_function",
    "baryonic_tully_fisher",
    "specific_sfr",
    "black_hole_bulge_relation",
    "gas_fraction",
    "metallicity",
    "bulge_mass_fraction",
    "quiescent_fraction",
    "halo_occupation",
    "baryon_fraction",
    "spin_distribution",
    "velocity_distribution",
    "mass_reservoir_scatter",
    "spatial_distribution",
]

"""List of all available evolution plot modules."""
EVOLUTION_PLOTS = [
    "smf_evolution",
    "sfr_density_evolution",
    "stellar_mass_density_evolution",
]

# Make sure this dictionary matches the classifications above

"""Mapping of plot names to their corresponding functions."""
PLOT_FUNCS = {
    "stellar_mass_function": stellar_mass_function.plot,
    "baryonic_mass_function": baryonic_mass_function.plot,
    "gas_mass_function": gas_mass_function.plot,
    "baryonic_tully_fisher": baryonic_tully_fisher.plot,
    "specific_sfr": specific_sfr.plot,
    "smf_evolution": smf_evolution.plot,
    "sfr_density_evolution": sfr_density_evolution.plot,
    "stellar_mass_density_evolution": stellar_mass_density_evolution.plot,
    "black_hole_bulge_relation": black_hole_bulge_relation.plot,
    "gas_fraction": gas_fraction.plot,
    "metallicity": metallicity.plot,
    "bulge_mass_fraction": bulge_mass_fraction.plot,
    "quiescent_fraction": quiescent_fraction.plot,
    "halo_occupation": halo_occupation.plot,
    "baryon_fraction": baryon_fraction.plot,
    "spin_distribution": spin_distribution.plot,
    "velocity_distribution": velocity_distribution.plot,
    "mass_reservoir_scatter": mass_reservoir_scatter.plot,
    "spatial_distribution": spatial_distribution.plot,
}
