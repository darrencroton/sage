/**
 * @file    model_reincorporation.c
 * @brief   Implementation of gas reincorporation from ejected reservoir
 *
 * This file implements the physical model for the reincorporation of gas
 * that was previously ejected from galaxies by powerful feedback events.
 * The reincorporation rate depends on the virial velocity of the halo,
 * with more massive halos able to recapture their ejected gas more efficiently.
 *
 * The model is based on the concept that supernova-driven winds can escape
 * the gravitational potential of low-mass halos but may be recaptured by
 * more massive halos. Gas reincorporation becomes effective when the halo's
 * escape velocity exceeds the characteristic velocity of supernova-driven
 * winds.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"
#include "util_numeric.h"

/**
 * @brief   Reincorporates gas from the ejected reservoir back into the hot halo
 *
 * @param   centralgal    Index of the central galaxy
 * @param   dt            Time step size
 *
 * This function models the reincorporation of gas that was previously ejected
 * from the galaxy due to supernova feedback. The reincorporation rate depends
 * on:
 *
 * 1. The ratio of the halo's virial velocity to a critical velocity (Vcrit)
 * 2. The amount of gas in the ejected reservoir
 * 3. The dynamical time of the halo (Rvir/Vvir)
 *
 * Reincorporation only occurs in halos with virial velocities exceeding the
 * critical velocity, which is related to the characteristic velocity of
 * supernova-driven winds. The rate increases for more massive halos (higher
 * Vvir).
 *
 * Metals are reincorporated along with the gas, preserving the metallicity of
 * the ejected reservoir.
 */
void reincorporate_gas(int centralgal, double dt) {
  double reincorporated, metallicity;

  /* Critical velocity for reincorporation
   * SN velocity is ~630km/s, and reincorporation occurs when the halo escape
   * velocity exceeds this value. The escape velocity relates to virial velocity
   * as V_esc = sqrt(2)*V_vir, so the critical virial velocity is V_SN/sqrt(2) =
   * 445.48 km/s This is modified by a tunable parameter ReIncorporationFactor
   */
  double Vcrit = 445.48 * SageConfig.ReIncorporationFactor;

  /* Only reincorporate gas if the halo virial velocity exceeds the critical
   * value */
  if (is_greater(Gal[centralgal].Vvir, Vcrit)) {
    /* Calculate reincorporation rate:
     * Rate = (Vvir/Vcrit - 1) * Mejected / tdyn * dt
     * Where tdyn is the dynamical time of the halo (Rvir/Vvir) */
    reincorporated =
        (safe_div(Gal[centralgal].Vvir, Vcrit, EPSILON_SMALL) - 1.0) *
        Gal[centralgal].EjectedMass *
        safe_div(Gal[centralgal].Vvir, Gal[centralgal].Rvir, EPSILON_SMALL) *
        dt;

    /* Limit reincorporation to the available ejected mass */
    if (is_greater(reincorporated, Gal[centralgal].EjectedMass))
      reincorporated = Gal[centralgal].EjectedMass;

    /* Calculate metallicity of ejected gas */
    metallicity = get_metallicity(Gal[centralgal].EjectedMass,
                                  Gal[centralgal].MetalsEjectedMass);

    /* Update galaxy components */
    Gal[centralgal].EjectedMass -=
        reincorporated; /* Remove from ejected reservoir */
    Gal[centralgal].MetalsEjectedMass -=
        metallicity * reincorporated; /* Remove metals */

    Gal[centralgal].HotGas += reincorporated; /* Add to hot gas reservoir */
    Gal[centralgal].MetalsHotGas +=
        metallicity * reincorporated; /* Add metals */
  }
}
