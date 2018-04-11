#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"



void starformation_and_feedback(int p, int centralgal, double time, double dt, int halonr, int step)
{
  double reff, tdyn, strdot, stars, reheated_mass, ejected_mass, fac, metallicity;
  double cold_crit;
  double FracZleaveDiskVal;
  
  // Initialise variables
  strdot = 0.0;

  // star formation recipes 
  if(SFprescription == 0)
  {
    // we take the typical star forming region as 3.0*r_s using the Milky Way as a guide
    reff = 3.0 * Gal[p].DiskScaleRadius;
    tdyn = reff / Gal[p].Vvir;

    // from Kauffmann (1996) eq7 x piR^2, (Vvir in km/s, reff in Mpc/h) in units of 10^10Msun/h 
    cold_crit = 0.19 * Gal[p].Vvir * reff;
		if(Gal[p].ColdGas > cold_crit && tdyn > 0.0)
			strdot = SfrEfficiency * (Gal[p].ColdGas - cold_crit) / tdyn;
		else
			strdot = 0.0;
  }
	else
	{
		printf("No star formation prescription selected!\n");
    ABORT(0);
	}

  stars = strdot * dt;
  if(stars < 0.0)
    stars = 0.0;

  if(SupernovaRecipeOn == 1)
    reheated_mass = FeedbackReheatingEpsilon * stars;
  else
    reheated_mass = 0.0;

	assert(reheated_mass >= 0.0);

  // cant use more cold gas than is available! so balance SF and feedback 
  if((stars + reheated_mass) > Gal[p].ColdGas && (stars + reheated_mass) > 0.0)
  {
    fac = Gal[p].ColdGas / (stars + reheated_mass);
    stars *= fac;
    reheated_mass *= fac;
  }

  // determine ejection
  if(SupernovaRecipeOn == 1)
  {
    if(Gal[centralgal].Vvir > 0.0)
			ejected_mass = 
				(FeedbackEjectionEfficiency * (EtaSNcode * EnergySNcode) / (Gal[centralgal].Vvir * Gal[centralgal].Vvir) -
					FeedbackReheatingEpsilon) * stars;
		else
			ejected_mass = 0.0;
		
    if(ejected_mass < 0.0)
      ejected_mass = 0.0;
  }
  else
    ejected_mass = 0.0;

  // update the star formation rate 
  Gal[p].SfrDisk[step] += stars / dt;
  Gal[p].SfrDiskColdGas[step] = Gal[p].ColdGas;
  Gal[p].SfrDiskColdGasMetals[step] = Gal[p].MetalsColdGas;

  // update for star formation 
  metallicity = get_metallicity(Gal[p].ColdGas, Gal[p].MetalsColdGas);
  update_from_star_formation(p, stars, metallicity);

  // recompute the metallicity of the cold phase
  metallicity = get_metallicity(Gal[p].ColdGas, Gal[p].MetalsColdGas);

  // update from SN feedback 
  update_from_feedback(p, centralgal, reheated_mass, ejected_mass, metallicity);

  // check for disk instability
  if(DiskInstabilityOn)
    check_disk_instability(p, centralgal, halonr, time, dt, step);

  // formation of new metals - instantaneous recycling approximation - only SNII 
  if(Gal[p].ColdGas > 1.0e-8)
  {
    FracZleaveDiskVal = FracZleaveDisk * exp(-1.0 * Gal[centralgal].Mvir / 30.0);  // Krumholz & Dekel 2011 Eq. 22
    Gal[p].MetalsColdGas += Yield * (1.0 - FracZleaveDiskVal) * stars;
    Gal[centralgal].MetalsHotGas += Yield * FracZleaveDiskVal * stars;
    // Gal[centralgal].MetalsEjectedMass += Yield * FracZleaveDiskVal * stars;
  }
  else
    Gal[centralgal].MetalsHotGas += Yield * stars;
    // Gal[centralgal].MetalsEjectedMass += Yield * stars;
}



void update_from_star_formation(int p, double stars, double metallicity)
{
  // update gas and metals from star formation 
  Gal[p].ColdGas -= (1 - RecycleFraction) * stars;
  Gal[p].MetalsColdGas -= metallicity * (1 - RecycleFraction) * stars;
  Gal[p].StellarMass += (1 - RecycleFraction) * stars;
  Gal[p].MetalsStellarMass += metallicity * (1 - RecycleFraction) * stars;
}



void update_from_feedback(int p, int centralgal, double reheated_mass, double ejected_mass, double metallicity)
{
  double metallicityHot;

	assert(!(reheated_mass > Gal[p].ColdGas && reheated_mass > 0.0));

  if(SupernovaRecipeOn == 1)
  {
    Gal[p].ColdGas -= reheated_mass;
    Gal[p].MetalsColdGas -= metallicity * reheated_mass;

    Gal[centralgal].HotGas += reheated_mass;
    Gal[centralgal].MetalsHotGas += metallicity * reheated_mass;

    if(ejected_mass > Gal[centralgal].HotGas)
      ejected_mass = Gal[centralgal].HotGas;
    metallicityHot = get_metallicity(Gal[centralgal].HotGas, Gal[centralgal].MetalsHotGas);

    Gal[centralgal].HotGas -= ejected_mass;
    Gal[centralgal].MetalsHotGas -= metallicityHot * ejected_mass;
    Gal[centralgal].EjectedMass += ejected_mass;
    Gal[centralgal].MetalsEjectedMass += metallicityHot * ejected_mass;

    Gal[p].OutflowRate += reheated_mass;    
  }

}


