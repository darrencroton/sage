"""
SAGE Figure Modules

This package contains self-contained modules for creating various plots from SAGE galaxy data.
"""

# Import all the figure modules so they can be discovered
from . import stellar_mass_function
from . import baryonic_mass_function
from . import gas_mass_function
from . import baryonic_tully_fisher
from . import specific_sfr
from . import smf_evolution
from . import sfr_density_evolution
from . import stellar_mass_density_evolution

# Define available plot types
SNAPSHOT_PLOTS = [
    'stellar_mass_function',
    'baryonic_mass_function',
    'gas_mass_function',
    'baryonic_tully_fisher',
    'specific_sfr'
]

EVOLUTION_PLOTS = [
    'smf_evolution',
    'sfr_density_evolution',
    'stellar_mass_density_evolution'
]

# Define a mapping of plot names to their functions
PLOT_FUNCS = {
    'stellar_mass_function': stellar_mass_function.plot,
    'baryonic_mass_function': baryonic_mass_function.plot,
    'gas_mass_function': gas_mass_function.plot,
    'baryonic_tully_fisher': baryonic_tully_fisher.plot,
    'specific_sfr': specific_sfr.plot,
    'smf_evolution': smf_evolution.plot,
    'sfr_density_evolution': sfr_density_evolution.plot,
    'stellar_mass_density_evolution': stellar_mass_density_evolution.plot
}
