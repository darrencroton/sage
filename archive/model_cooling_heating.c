/**
 * @file    model_cooling_heating.c
 * @brief   Gas cooling and AGN heating processes for galaxy evolution
 *
 * This file implements the physical processes governing gas cooling from the
 * hot halo onto the cold disk, and the counteracting heating from AGN feedback.
 * The cooling model is based on an isothermal density profile with
 * metallicity-dependent cooling functions, while the AGN heating model
 * simulates the effects of energy injection from the central supermassive black
 * hole.
 *
 * Key functions:
 * - cooling_recipe(): Calculates gas cooling rate based on halo properties
 * - do_AGN_heating(): Implements AGN feedback and black hole accretion
 * - cool_gas_onto_galaxy(): Transfers cooled gas from hot halo to cold disk
 *
 * References:
 * - White & Frenk (1991) - Cooling model framework
 * - Croton et al. (2006) - AGN feedback implementation
 * - Kauffmann et al. (1999) - Cooling implementation
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"
#include "util_numeric.h"

/**
 * @brief   Calculates gas cooling based on halo properties and cooling
 * functions
 *
 * @param   gal    Index of the galaxy in the Gal array
 * @param   dt     Time step size
 * @return  Mass of gas that cools from hot to cold phase in this time step
 *
 * This function implements the standard cooling model where hot gas cools
 * from an isothermal density profile based on a cooling radius. The cooling
 * rate depends on the gas temperature (determined by the virial velocity),
 * the gas metallicity, and the corresponding cooling function.
 *
 * Two cooling regimes are considered:
 * 1. "Cold accretion" when rcool > Rvir: rapid cooling throughout the halo
 * 2. "Hot halo cooling" when rcool < Rvir: cooling only within cooling radius
 *
 * If AGN feedback is enabled, the cooling rate can be reduced by past
 * heating events.
 */
double cooling_recipe(int gal, double dt) {
  double tcool, x, logZ, lambda, rcool, rho_rcool, rho0, temp, coolingGas;

  /* Only proceed if the galaxy has hot gas and a non-zero virial velocity */
  if (is_greater(Gal[gal].HotGas, 0.0) && is_greater(Gal[gal].Vvir, 0.0)) {
    /* Calculate dynamical time of the halo (approximation for cooling time) */
    tcool = Gal[gal].Rvir / Gal[gal].Vvir;

    /* Calculate virial temperature from virial velocity
     * T = 35.9 * V^2 where V is in km/s and T is in Kelvin
     * This comes from T = (μ*m_p*V^2)/(2*k_B) */
    temp = 35.9 * Gal[gal].Vvir * Gal[gal].Vvir;

    /* Calculate log of metallicity (Z/Z_sun) for cooling function lookup */
    if (is_greater(Gal[gal].MetalsHotGas, 0.0))
      logZ = log10(
          safe_div(Gal[gal].MetalsHotGas, Gal[gal].HotGas, EPSILON_SMALL));
    else
      logZ = -10.0; /* Set very low metallicity if no metals */

    /* Get cooling rate (lambda) from interpolation tables based on temperature
     * and metallicity */
    lambda = get_metaldependent_cooling_rate(log10(temp), logZ);

    /* Calculate coefficient for cooling density threshold
     * x = (m_p * k_B * T) / lambda in physical units (sec * g/cm^3) */
    x = PROTONMASS * BOLTZMANN * temp / lambda;

    /* Convert to simulation units */
    x /= (UnitDensity_in_cgs * UnitTime_in_s);

    /* Calculate density at cooling radius
     * Factor 0.885 = 3/2 * mu, where mu=0.59 for fully ionized gas
     * This is the density where cooling time equals dynamical time */
    rho_rcool = x / tcool * 0.885;

    /* Calculate central density assuming isothermal profile for hot gas */
    rho0 = Gal[gal].HotGas / (4 * M_PI * Gal[gal].Rvir);

    /* Calculate cooling radius where tcool = tdyn */
    rcool = sqrt(rho0 / rho_rcool);

    /* Determine cooling regime and calculate cooled gas mass */
    if (is_greater(rcool, Gal[gal].Rvir))
      /* "Cold accretion" regime - rapid cooling throughout the halo
       * All hot gas cools on the dynamical timescale */
      coolingGas = Gal[gal].HotGas *
                   safe_div(Gal[gal].Vvir, Gal[gal].Rvir, EPSILON_SMALL) * dt;
    else
      /* "Hot halo cooling" regime - cooling only within cooling radius
       * This follows from integrating the isothermal density profile
       * within rcool and dividing by the cooling time */
      coolingGas = safe_div(Gal[gal].HotGas, Gal[gal].Rvir, EPSILON_SMALL) *
                   safe_div(rcool, (2.0 * tcool), EPSILON_SMALL) * dt;

    /* Apply limits to ensure physically sensible cooling */
    if (is_greater(coolingGas, Gal[gal].HotGas))
      coolingGas = Gal[gal].HotGas; /* Cannot cool more gas than is available */
    else if (is_less_or_equal(coolingGas, 0.0))
      coolingGas = 0.0; /* Prevent negative cooling */

    /* At this point we have calculated the maximal cooling rate
     * If AGN feedback is enabled, reduce cooling based on past heating */
    if (SageConfig.AGNrecipeOn > 0 && is_greater(coolingGas, 0.0))
      coolingGas = do_AGN_heating(coolingGas, gal, dt, x, rcool);

    /* Accumulate cooling energy for energy budget calculation
     * E_cool = 0.5 * m * V_vir^2 = change in potential energy */
    if (is_greater(coolingGas, 0.0))
      Gal[gal].Cooling += 0.5 * coolingGas * Gal[gal].Vvir * Gal[gal].Vvir;
  } else
    coolingGas = 0.0; /* No cooling if no hot gas or zero virial velocity */

  assert(is_greater_or_equal(coolingGas, 0.0)); /* Ensure no negative cooling */
  return coolingGas;
}

/**
 * @brief   Implements AGN heating and black hole accretion process
 *
 * @param   coolingGas    Current calculated cooling gas mass
 * @param   centralgal    Index of the central galaxy in the Gal array
 * @param   dt            Time step size
 * @param   x             Cooling coefficient (from cooling_recipe)
 * @param   rcool         Cooling radius (from cooling_recipe)
 * @return  Updated cooling gas mass after accounting for AGN heating
 *
 * This function models the suppression of cooling by AGN feedback and
 * the growth of the central supermassive black hole. It implements:
 *
 * 1. Reduction of cooling based on past heating events
 * 2. Black hole accretion through one of three selectable methods:
 *    - Empirical scaling with BH mass, halo velocity, and gas fraction
 *    - Bondi-Hoyle accretion based on gas density and BH mass
 *    - Cold cloud accretion triggered by BH mass threshold
 * 3. Limiting of accretion by the Eddington rate
 * 4. Tracking of the heating radius and energy
 *
 * The function returns the updated cooling gas mass after heating effects.
 */
double do_AGN_heating(double coolingGas, int centralgal, double dt, double x,
                      double rcool) {
  double AGNrate, EDDrate, AGNaccreted, AGNcoeff, AGNheating, metallicity,
      r_heat_new;

  /* First, reduce cooling rate based on past AGN heating events
   * This models the cumulative effect of multiple AGN outbursts */
  if (is_less(Gal[centralgal].r_heat, rcool))
    coolingGas =
        (1.0 - safe_div(Gal[centralgal].r_heat, rcool, 1.0)) * coolingGas;
  else
    coolingGas =
        0.0; /* Complete suppression if heating radius exceeds cooling radius */

  assert(is_greater_or_equal(coolingGas, 0.0));

  /* Calculate the new heating rate from black hole accretion */
  if (is_greater(Gal[centralgal].HotGas, 0.0)) {
    /* Choose accretion model based on configuration */
    if (SageConfig.AGNrecipeOn == 2) {
      /* Bondi-Hoyle accretion recipe
       * Based on BH mass and local gas properties
       * Formula: AGNrate ~ G * rho * M_BH^2 / c_s^3
       * Where: 2.5*π*G = gravitational constant term
       *        0.375*0.6*x = gas density and sound speed term
       *        M_BH = black hole mass
       *        RadioModeEfficiency = overall efficiency parameter */
      AGNrate = (2.5 * M_PI * G) * (0.375 * 0.6 * x) *
                Gal[centralgal].BlackHoleMass * SageConfig.RadioModeEfficiency;
    } else if (SageConfig.AGNrecipeOn == 3) {
      /* Cold cloud accretion model
       * Triggered when BH mass exceeds threshold related to cooling properties
       * Accretion rate = 0.01% of cooling rate when triggered */
      if (is_greater(
              Gal[centralgal].BlackHoleMass,
              0.0001 * Gal[centralgal].Mvir *
                  pow(safe_div(rcool, Gal[centralgal].Rvir, EPSILON_SMALL),
                      3.0)))
        AGNrate = 0.0001 * safe_div(coolingGas, dt, EPSILON_SMALL);
      else
        AGNrate = 0.0;
    } else {
      /* Empirical (standard) accretion recipe
       * Scales with black hole mass, virial velocity, and hot gas fraction
       * Formula based on simulation fits rather than physical first principles
       */
      if (is_greater(Gal[centralgal].Mvir, 0.0))
        AGNrate = SageConfig.RadioModeEfficiency /
                  (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS) *
                  (Gal[centralgal].BlackHoleMass / 0.01) *
                  pow(Gal[centralgal].Vvir / 200.0, 3.0) *
                  (safe_div(Gal[centralgal].HotGas, Gal[centralgal].Mvir,
                            EPSILON_SMALL) /
                   0.1);
      else
        AGNrate = SageConfig.RadioModeEfficiency /
                  (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS) *
                  (Gal[centralgal].BlackHoleMass / 0.01) *
                  pow(Gal[centralgal].Vvir / 200.0, 3.0);
    }

    /* Calculate Eddington accretion rate limit
     * L_Edd = 1.3e38 * (M_BH/M_sun) erg/s
     * Convert to mass accretion rate using E = 0.1 * m * c^2 efficiency */
    EDDrate =
        (1.3e38 * Gal[centralgal].BlackHoleMass * 1e10 / SageConfig.Hubble_h) /
        (UnitEnergy_in_cgs / UnitTime_in_s) / (0.1 * 9e10);

    /* Limit accretion to Eddington rate */
    if (is_greater(AGNrate, EDDrate))
      AGNrate = EDDrate;

    /* Calculate total mass accreted onto black hole in this time step */
    AGNaccreted = AGNrate * dt;

    /* Ensure we don't accrete more hot gas than is available */
    if (is_greater(AGNaccreted, Gal[centralgal].HotGas))
      AGNaccreted = Gal[centralgal].HotGas;

    /* Calculate heating efficiency coefficient
     * This represents energy per unit mass needed to heat gas to virial
     * temperature 1.34e5 = sqrt(2*eta*c^2), where eta=0.1 is standard
     * efficiency and c is speed of light in km/s Dividing by Vvir converts to
     * dimensionless heating efficiency */
    AGNcoeff = pow(safe_div(1.34e5, Gal[centralgal].Vvir, EPSILON_SMALL), 2.0);

    /* Calculate mass of cooling gas that can be suppressed by this heating */
    AGNheating = AGNcoeff * AGNaccreted;

    /* Limit heating to the current cooling rate to maintain energy conservation
     * If heating would exceed cooling, reduce accretion accordingly */
    if (is_greater(AGNheating, coolingGas)) {
      AGNaccreted = coolingGas / AGNcoeff;
      AGNheating = coolingGas;
    }

    /* Update galaxy properties based on black hole accretion */
    metallicity =
        get_metallicity(Gal[centralgal].HotGas, Gal[centralgal].MetalsHotGas);
    Gal[centralgal].BlackHoleMass += AGNaccreted; /* Grow the black hole */
    Gal[centralgal].HotGas -=
        AGNaccreted; /* Remove accreted gas from hot phase */
    Gal[centralgal].MetalsHotGas -=
        metallicity * AGNaccreted; /* Remove corresponding metals */

    /* Update the heating radius - this affects future cooling suppression
     * The heating radius grows when effective heating occurs */
    if (is_less(Gal[centralgal].r_heat, rcool) && is_greater(coolingGas, 0.0)) {
      r_heat_new = safe_div(AGNheating, coolingGas, 0.0) * rcool;
      if (is_greater(r_heat_new, Gal[centralgal].r_heat))
        Gal[centralgal].r_heat = r_heat_new;
    }

    /* Track heating energy for energy budget calculations
     * E_heat = 0.5 * m * V_vir^2 = energy needed to heat gas to virial
     * temperature */
    if (is_greater(AGNheating, 0.0))
      Gal[centralgal].Heating +=
          0.5 * AGNheating * Gal[centralgal].Vvir * Gal[centralgal].Vvir;
  }

  return coolingGas; /* Return updated cooling gas mass after heating effects */
}

/**
 * @brief   Transfers cooled gas from the hot halo to the cold disk
 *
 * @param   centralgal    Index of the central galaxy in the Gal array
 * @param   coolingGas    Mass of gas to be transferred from hot to cold phase
 *
 * This function moves the calculated amount of cooling gas from the hot
 * halo to the cold disk of the galaxy, along with its associated metals.
 * It ensures that we don't try to cool more gas than is available in the
 * hot component.
 *
 * The function is called for each time step after calculating the cooling
 * rate and applying any AGN heating effects.
 */
void cool_gas_onto_galaxy(int centralgal, double coolingGas) {
  double metallicity;

  /* Only proceed if there is gas to cool */
  if (is_greater(coolingGas, 0.0)) {
    /* Check if we're trying to cool more gas than is available in the hot halo
     */
    if (is_less(coolingGas, Gal[centralgal].HotGas)) {
      /* Normal case: cooling doesn't deplete hot gas completely */
      metallicity =
          get_metallicity(Gal[centralgal].HotGas, Gal[centralgal].MetalsHotGas);

      /* Add cooled gas to cold disk */
      Gal[centralgal].ColdGas += coolingGas;
      Gal[centralgal].MetalsColdGas += metallicity * coolingGas;

      /* Remove cooled gas from hot halo */
      Gal[centralgal].HotGas -= coolingGas;
      Gal[centralgal].MetalsHotGas -= metallicity * coolingGas;
    } else {
      /* Edge case: cooling would deplete hot gas completely
       * Transfer all remaining hot gas to cold disk */
      Gal[centralgal].ColdGas += Gal[centralgal].HotGas;
      Gal[centralgal].MetalsColdGas += Gal[centralgal].MetalsHotGas;

      /* Reset hot gas to zero */
      Gal[centralgal].HotGas = 0.0;
      Gal[centralgal].MetalsHotGas = 0.0;
    }
  }
}
