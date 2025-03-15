/**
 * @file    core_cool_func.c
 * @brief   Cooling function implementation for gas in dark matter halos
 *
 * This file implements the cooling rate calculations for hot gas in dark matter
 * halos. It reads tabulated cooling functions from data files based on
 * metallicity and temperature, and provides interpolation functions to obtain
 * cooling rates for arbitrary temperature and metallicity values.
 *
 * The cooling tables are based on collisional ionization equilibrium models and
 * cover a range of metallicities from primordial composition to super-solar.
 * Temperature coverage is from 10^4 K to 10^8.5 K (in log steps of 0.05 dex).
 *
 * Key functions:
 * - read_cooling_functions(): Loads cooling tables from files
 * - get_metaldependent_cooling_rate(): Interpolates cooling rate based on
 * temperature and metallicity
 * - get_rate(): Helper function for table lookup and temperature interpolation
 *
 * References:
 * - Sutherland & Dopita (1993) - Cooling function tables
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"

#define TABSIZE 91

static char *name[] = {"stripped_mzero.cie", "stripped_m-30.cie",
                       "stripped_m-20.cie",  "stripped_m-15.cie",
                       "stripped_m-10.cie",  "stripped_m-05.cie",
                       "stripped_m-00.cie",  "stripped_m+05.cie"};

// Metallicies with repect to solar. Will be converted to absolut metallicities
// by adding log10(Z_sun), Zsun=0.02
static double metallicities[8] = {-5.0, // actually primordial -> -infinity
                                  -3.0, -2.0, -1.5, -1.0, -0.5, +0.0, +0.5};

static double CoolRate[8][TABSIZE];

/**
 * @brief   Reads tabulated cooling functions from data files
 *
 * This function loads the cooling function tables from external data files.
 * It reads eight files corresponding to different metallicity values, from
 * primordial composition to super-solar metallicity. Each file contains
 * tabulated cooling rates as a function of temperature.
 *
 * The cooling rates are stored in the CoolRate array for later interpolation.
 * The function also converts metallicities from [Fe/H] notation (relative to
 * solar) to absolute metallicities by adding log10(Z_sun), where Z_sun=0.02.
 *
 * The cooling tables cover a temperature range from 10^4 K to 10^8.5 K
 * with 91 temperature points in steps of 0.05 dex in log(T).
 */
void read_cooling_functions(void) {
  FILE *fd;
  char buf[200];
  int i, n;
  float sd_logT, sd_ne, sd_nh, sd_nt, sd_logLnet, sd_logLnorm, sd_logU,
      sd_logTau, sd_logP12, sd_logRho24, sd_ci, sd_mubar;

  for (i = 0; i < 8; i++)
    metallicities[i] += log10(0.02); // add solar metallicity

  for (i = 0; i < 8; i++) {
    sprintf(buf, "extra/CoolFunctions/%s", name[i]);

    if (!(fd = fopen(buf, "r"))) {
      fprintf(stderr,
              "Error: Cooling function file '%s' not found. Please ensure the "
              "extra/CoolFunctions directory contains all required cooling "
              "tables.\n",
              buf);
      ABORT(0);
    }

    for (n = 0; n <= 90; n++) {
      fscanf(fd, " %f %f %f %f %f %f %f %f %f %f %f %f ", &sd_logT, &sd_ne,
             &sd_nh, &sd_nt, &sd_logLnet, &sd_logLnorm, &sd_logU, &sd_logTau,
             &sd_logP12, &sd_logRho24, &sd_ci, &sd_mubar);

      CoolRate[i][n] = sd_logLnorm;
    }

    fclose(fd);
  }

#ifdef MPI
  if (ThisTask == 0)
#endif
    INFO_LOG("Cooling functions successfully read from %d files", 8);
}

/**
 * @brief   Calculates cooling rate based on temperature and metallicity
 *
 * @param   logTemp   Log10 of gas temperature in Kelvin
 * @param   logZ      Log10 of gas metallicity (absolute, not [Fe/H])
 * @return  Cooling rate in units of erg cm^3 s^-1
 *
 * This function performs a 2D interpolation of the cooling rate tables
 * to determine the cooling rate for an arbitrary temperature and metallicity.
 * It first identifies the appropriate metallicity bracket, then interpolates
 * between cooling rates at that temperature for the two bracketing
 * metallicities.
 *
 * The function enforces limits on the metallicity range:
 * - If logZ < minimum tabulated value (primordial), uses primordial cooling
 * - If logZ > maximum tabulated value (super-solar), uses super-solar cooling
 *
 * Returns the cooling rate in physical units (erg cm^3 s^-1), which can
 * be used to calculate the cooling time when divided by temperature.
 */
double get_metaldependent_cooling_rate(
    double logTemp,
    double logZ) // pass: log10(temperatue/Kelvin), log10(metallicity)
{
  int i;
  double get_rate(int tab, double logTemp);
  double rate1, rate2, rate;

  if (logZ < metallicities[0])
    logZ = metallicities[0];

  if (logZ > metallicities[7])
    logZ = metallicities[7];

  i = 0;
  while (logZ > metallicities[i + 1])
    i++;

  // look up at i and i+1
  rate1 = get_rate(i, logTemp);
  rate2 = get_rate(i + 1, logTemp);

  rate = rate1 + (rate2 - rate1) / (metallicities[i + 1] - metallicities[i]) *
                     (logZ - metallicities[i]);

  return pow(10, rate);
}

/**
 * @brief   Helper function for temperature interpolation in cooling tables
 *
 * @param   tab      Index of the metallicity table to use
 * @param   logTemp  Log10 of gas temperature in Kelvin
 * @return  Log10 of cooling rate for the specified metallicity and temperature
 *
 * This function performs the temperature interpolation within a single
 * metallicity cooling table. It:
 * 1. Ensures temperature is within valid range (10^4 K to 10^8.5 K)
 * 2. Identifies the temperature bin in the cooling table
 * 3. Performs linear interpolation between adjacent table entries
 *
 * Returns the log10 of the cooling rate, which is then used by
 * get_metaldependent_cooling_rate() for the full 2D interpolation.
 */
double get_rate(int tab, double logTemp) {
  int index;
  double rate1, rate2, rate, logTindex;

  if (logTemp < 4.0)
    logTemp = 4.0;

  index = (logTemp - 4.0) / 0.05;
  if (index >= 90)
    index = 89;

  logTindex = 4.0 + 0.05 * index;

  rate1 = CoolRate[tab][index];
  rate2 = CoolRate[tab][index + 1];

  rate = rate1 + (rate2 - rate1) / (0.05) * (logTemp - logTindex);

  return rate;
}
