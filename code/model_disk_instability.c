/**
 * @file    model_disk_instability.c
 * @brief   Implementation of disk instability detection and response
 *
 * This file implements the model for disk instability in galaxies, which occurs
 * when the self-gravity of a galactic disk dominates over the gravitational
 * support from the dark matter halo. When instability is detected, mass is
 * transferred from the disk to the bulge to restore stability.
 *
 * The stability criterion follows Mo, Mao & White (1998), comparing the disk
 * mass to a critical mass dependent on the maximum circular velocity and disk
 * scale radius. When the disk mass exceeds this critical value, the excess
 * mass (both stars and gas) is transferred to the bulge, with the gas
 * component triggering a starburst and potential black hole growth.
 *
 * Reference:
 * - Mo, Mao & White (1998) for disk stability criterion
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



/**
 * @brief   Checks for disk instability and transfers mass to restore stability
 *
 * @param   p             Index of the galaxy in the Gal array
 * @param   centralgal    Index of the central galaxy
 * @param   halonr        Index of the current halo
 * @param   time          Current time in the simulation
 * @param   dt            Time step size
 * @param   step          Current substep in the time integration
 *
 * This function evaluates the stability of galactic disks using the criterion
 * from Mo, Mao & White (1998). If a disk is found to be unstable, the excess
 * mass is transferred to the bulge component:
 * 
 * 1. Stellar disk mass goes directly to the stellar bulge
 * 2. Cold gas triggers a starburst (forming stars that go to the bulge)
 * 3. If AGN is enabled, some gas can fuel black hole growth
 *
 * The stability criterion compares the disk mass to a critical mass:
 * Mcrit = Vmax^2 * (3 * DiskScaleRadius) / G
 * 
 * The function maintains appropriate tracking of metals during all transfers.
 */
void check_disk_instability(int p, int centralgal, int halonr, double time, double dt, int step)
{
  double Mcrit, gas_fraction, unstable_gas, unstable_gas_fraction, unstable_stars, diskmass, metallicity;
  double star_fraction;

  /* Calculate the stability of the stellar and gaseous disk following Mo, Mao & White (1998).
   * For unstable stars and gas, we transfer the required amount to the bulge to restore stability */

  /* Calculate total disk mass (cold gas + stellar disk) */
  diskmass = Gal[p].ColdGas + (Gal[p].StellarMass - Gal[p].BulgeMass);
  
  /* Only proceed if disk mass is positive */
  if(diskmass > 0.0)
  {
    /* Calculate critical disk mass for stability:
     * Mcrit = Vmax^2 * (3 * DiskScaleRadius) / G
     * This is derived from requiring that the disk's self-gravity doesn't dominate */
    Mcrit = Gal[p].Vmax * Gal[p].Vmax * (3.0 * Gal[p].DiskScaleRadius) / G;
    
    /* Limit critical mass to actual disk mass (can't have negative unstable mass) */
    if(Mcrit > diskmass)
      Mcrit = diskmass;
    
    /* Calculate the fractions of gas and stars in the disk */
    gas_fraction   = Gal[p].ColdGas / diskmass;
    star_fraction  = 1.0 - gas_fraction;
    
    /* Calculate unstable gas and stellar masses that need to be transferred */
    unstable_gas   = gas_fraction * (diskmass - Mcrit);
    unstable_stars = star_fraction * (diskmass - Mcrit);

    /* Handle unstable stars - transfer directly to the bulge */
    if(unstable_stars > 0.0)
    {
      /* Calculate disk stellar metallicity (excluding existing bulge) */
      metallicity = get_metallicity(Gal[p].StellarMass - Gal[p].BulgeMass, 
                                    Gal[p].MetalsStellarMass - Gal[p].MetalsBulgeMass);

      /* Add unstable stars to the bulge, preserving metallicity */
      Gal[p].BulgeMass += unstable_stars;
      Gal[p].MetalsBulgeMass += metallicity * unstable_stars;
      
      /* Merge tracking code commented out in original
       * Need to fix this. Excluded for now. */
      // Gal[p].mergeType = 3;  // mark as disk instability partial mass transfer
      // Gal[p].mergeIntoID = NumGals + p - 1;      
      
      /* Sanity check to ensure bulge mass doesn't exceed total stellar mass */
      if (Gal[p].BulgeMass / Gal[p].StellarMass > 1.0001 || 
          Gal[p].MetalsBulgeMass / Gal[p].MetalsStellarMass > 1.0001)
      {
        WARNING_LOG("Disk instability caused bulge mass to exceed total stellar mass in galaxy %d. Bulge/Total = %.4f (stars) or %.4f (metals)",
                    p, Gal[p].BulgeMass / Gal[p].StellarMass, Gal[p].MetalsBulgeMass / Gal[p].MetalsStellarMass);
        // ABORT(0);  /* Error checking disabled in original */
      }
    }

    /* Handle unstable gas - trigger starburst and black hole growth */
    if(unstable_gas > 0.0)
    {
      /* Sanity check to ensure unstable gas doesn't exceed available cold gas */
      if(unstable_gas/Gal[p].ColdGas > 1.0001)
      {
        WARNING_LOG("Disk instability calculation produced unstable gas mass exceeding total cold gas in galaxy %d. Unstable gas = %.4e, Cold gas = %.4e", 
                    p, unstable_gas, Gal[p].ColdGas);
        // ABORT(0);  /* Error checking disabled in original */
      }

      /* Calculate fraction of cold gas that is unstable */
      unstable_gas_fraction = unstable_gas / Gal[p].ColdGas;
      
      /* Feed black hole if AGN recipe is enabled */
      if(SageConfig.AGNrecipeOn > 0)
        grow_black_hole(p, unstable_gas_fraction);
    
      /* Trigger a starburst with the unstable gas
       * Mode 1 indicates the burst is due to disk instability */
      collisional_starburst_recipe(unstable_gas_fraction, p, centralgal, 
                                  time, dt, halonr, 1, step);
    }
  }
}
