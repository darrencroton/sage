#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "globals.h"
#include "types.h"
#include "config.h"
#include "core_proto.h"



/**
 * @brief   Implements star formation and feedback processes for a galaxy
 *
 * @param   p             Index of the galaxy in the Gal array
 * @param   centralgal    Index of the central galaxy of the halo
 * @param   time          Current time in the simulation
 * @param   dt            Time step size
 * @param   halonr        Index of the current halo
 * @param   step          Current substep in the time integration
 *
 * This function is the main driver for star formation and feedback processes.
 * It calculates:
 * 1. Star formation rate based on cold gas content and disk properties
 * 2. Actual stellar mass formed during this time step
 * 3. Gas reheating due to supernova feedback
 * 4. Gas ejection from the halo due to strong feedback
 * 5. Metal enrichment from newly formed stars
 *
 * The star formation follows the Kennicutt-Schmidt law with a critical
 * gas surface density threshold. Feedback is proportional to the star
 * formation rate, following the supernova energy-driven model.
 */
void starformation_and_feedback(int p, int centralgal, double time, double dt, int halonr, int step)
{
  double reff, tdyn, strdot, stars, reheated_mass, ejected_mass, fac, metallicity;
  double cold_crit;
  double FracZleaveDiskVal;
  
  /* Initialize star formation rate */
  strdot = 0.0;

  /* Apply the selected star formation recipe */
  if(SageConfig.SFprescription == 0)
  {
    /* Calculate effective star-forming radius - 3 scale lengths (Milky Way guide) */
    reff = 3.0 * Gal[p].DiskScaleRadius;
    
    /* Calculate dynamical time of the disk */
    tdyn = reff / Gal[p].Vvir;

    /* Calculate critical cold gas mass using Kauffmann (1996) eq7 x πR²
     * (Vvir in km/s, reff in Mpc/h) in units of 10^10Msun/h */
    cold_crit = 0.19 * Gal[p].Vvir * reff;
    
    /* Star formation occurs only if gas mass exceeds critical threshold
     * and dynamical time is positive */
    if(Gal[p].ColdGas > cold_crit && tdyn > 0.0)
      strdot = SageConfig.SfrEfficiency * (Gal[p].ColdGas - cold_crit) / tdyn;
    else
      strdot = 0.0;
  }
  else
  {
    printf("No star formation prescription selected!\n");
    ABORT(0);
  }

  /* Calculate mass of stars formed in this time step */
  stars = strdot * dt;
  if(stars < 0.0)
    stars = 0.0;

  /* Calculate gas reheated by supernova feedback */
  if(SageConfig.SupernovaRecipeOn == 1)
    reheated_mass = SageConfig.FeedbackReheatingEpsilon * stars;
  else
    reheated_mass = 0.0;

  assert(reheated_mass >= 0.0);

  /* Ensure total gas used (stars + feedback) doesn't exceed available cold gas
   * If needed, scale down both star formation and feedback proportionally */
  if((stars + reheated_mass) > Gal[p].ColdGas && (stars + reheated_mass) > 0.0)
  {
    fac = Gal[p].ColdGas / (stars + reheated_mass);
    stars *= fac;
    reheated_mass *= fac;
  }

  /* Calculate gas ejection due to powerful feedback
   * This is the energy-driven outflow model where ejection depends on:
   * 1. Supernova energy per unit stellar mass (EtaSNcode * EnergySNcode)
   * 2. Halo escape velocity (proportional to Vvir²)
   * 3. Efficiency parameters */
  if(SageConfig.SupernovaRecipeOn == 1)
  {
    if(Gal[centralgal].Vvir > 0.0)
      ejected_mass = 
        (SageConfig.FeedbackEjectionEfficiency * (EtaSNcode * EnergySNcode) / (Gal[centralgal].Vvir * Gal[centralgal].Vvir) -
          SageConfig.FeedbackReheatingEpsilon) * stars;
    else
      ejected_mass = 0.0;
    
    /* Ensure ejected mass is non-negative */
    if(ejected_mass < 0.0)
      ejected_mass = 0.0;
  }
  else
    ejected_mass = 0.0;

  /* Update star formation rate history */
  Gal[p].SfrDisk[step] += stars / dt;
  Gal[p].SfrDiskColdGas[step] = Gal[p].ColdGas;
  Gal[p].SfrDiskColdGasMetals[step] = Gal[p].MetalsColdGas;

  /* Update galaxy properties from star formation */
  metallicity = get_metallicity(Gal[p].ColdGas, Gal[p].MetalsColdGas);
  update_from_star_formation(p, stars, metallicity);

  /* Recompute the metallicity of the cold phase after star formation */
  metallicity = get_metallicity(Gal[p].ColdGas, Gal[p].MetalsColdGas);

  /* Update galaxy properties from supernova feedback */
  update_from_feedback(p, centralgal, reheated_mass, ejected_mass, metallicity);

  /* Check for disk instability if enabled */
  if(SageConfig.DiskInstabilityOn)
    check_disk_instability(p, centralgal, halonr, time, dt, step);

  /* Formation of new metals - instantaneous recycling approximation
   * This models metal production from Type II supernovae */
  if(Gal[p].ColdGas > 1.0e-8)  /* Only if there is cold gas to enrich */
  {
    /* Calculate fraction of metals that leave the disk based on halo mass
     * Following Krumholz & Dekel 2011 Eq. 22 */
    FracZleaveDiskVal = SageConfig.FracZleaveDisk * exp(-1.0 * Gal[centralgal].Mvir / 30.0);
    
    /* Distribute newly produced metals between cold disk gas and hot halo gas */
    Gal[p].MetalsColdGas += SageConfig.Yield * (1.0 - FracZleaveDiskVal) * stars;
    Gal[centralgal].MetalsHotGas += SageConfig.Yield * FracZleaveDiskVal * stars;
    /* Commented out in original code: */
    /* Gal[centralgal].MetalsEjectedMass += SageConfig.Yield * FracZleaveDiskVal * stars; */
  }
  else
    /* If no cold gas, all metals go to hot gas */
    Gal[centralgal].MetalsHotGas += SageConfig.Yield * stars;
    /* Commented out in original code: */
    /* Gal[centralgal].MetalsEjectedMass += Yield * stars; */
}



/**
 * @brief   Updates galaxy properties due to star formation
 *
 * @param   p             Index of the galaxy in the Gal array
 * @param   stars         Mass of stars formed in this time step
 * @param   metallicity   Current metallicity of the cold gas
 *
 * This function implements the changes to galaxy properties caused by
 * star formation. It:
 * 1. Reduces cold gas mass (accounting for recycling)
 * 2. Reduces cold gas metal content
 * 3. Increases stellar mass
 * 4. Increases stellar metal content
 *
 * The recycling fraction (RecycleFraction) represents the portion of
 * stellar mass that is returned to the ISM immediately through stellar
 * winds and supernovae. This implements a simple form of the instantaneous
 * recycling approximation.
 */
void update_from_star_formation(int p, double stars, double metallicity)
{
  /* Update cold gas mass, accounting for recycling */
  Gal[p].ColdGas -= (1 - SageConfig.RecycleFraction) * stars;
  
  /* Update cold gas metal content */
  Gal[p].MetalsColdGas -= metallicity * (1 - SageConfig.RecycleFraction) * stars;
  
  /* Update stellar mass */
  Gal[p].StellarMass += (1 - SageConfig.RecycleFraction) * stars;
  
  /* Update stellar metal content */
  Gal[p].MetalsStellarMass += metallicity * (1 - SageConfig.RecycleFraction) * stars;
}



/**
 * @brief   Updates galaxy properties due to supernova feedback
 *
 * @param   p                Index of the galaxy in the Gal array
 * @param   centralgal       Index of the central galaxy of the halo
 * @param   reheated_mass    Mass of cold gas reheated to hot phase
 * @param   ejected_mass     Mass of hot gas ejected from the halo
 * @param   metallicity      Current metallicity of the cold gas
 *
 * This function implements the changes to galaxy properties caused by
 * supernova feedback. It handles:
 * 1. Reheating of cold gas to the hot phase (within the halo)
 * 2. Ejection of hot gas from the halo (to the ejected reservoir)
 * 3. Transfer of metals between the different gas phases
 * 4. Updating the outflow rate for the galaxy
 *
 * The supernova feedback follows a two-stage process:
 * - First, cold gas is reheated and added to the hot gas of the central galaxy
 * - Then, some of this hot gas may be ejected from the halo completely if
 *   the supernova energy is sufficient to overcome the halo potential
 */
void update_from_feedback(int p, int centralgal, double reheated_mass, double ejected_mass, double metallicity)
{
  double metallicityHot;

  /* Sanity check: reheated mass shouldn't exceed available cold gas */
  assert(!(reheated_mass > Gal[p].ColdGas && reheated_mass > 0.0));

  if(SageConfig.SupernovaRecipeOn == 1)
  {
    /* Remove reheated gas from cold phase */
    Gal[p].ColdGas -= reheated_mass;
    Gal[p].MetalsColdGas -= metallicity * reheated_mass;

    /* Add reheated gas to hot phase of central galaxy */
    Gal[centralgal].HotGas += reheated_mass;
    Gal[centralgal].MetalsHotGas += metallicity * reheated_mass;

    /* Limit ejected mass to available hot gas */
    if(ejected_mass > Gal[centralgal].HotGas)
      ejected_mass = Gal[centralgal].HotGas;
    
    /* Calculate current hot gas metallicity */
    metallicityHot = get_metallicity(Gal[centralgal].HotGas, Gal[centralgal].MetalsHotGas);

    /* Remove ejected gas from hot phase */
    Gal[centralgal].HotGas -= ejected_mass;
    Gal[centralgal].MetalsHotGas -= metallicityHot * ejected_mass;
    
    /* Add ejected gas to ejected reservoir */
    Gal[centralgal].EjectedMass += ejected_mass;
    Gal[centralgal].MetalsEjectedMass += metallicityHot * ejected_mass;

    /* Update outflow rate for the galaxy */
    Gal[p].OutflowRate += reheated_mass;    
  }
}


