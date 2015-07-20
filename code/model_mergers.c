#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"



double estimate_merging_time(int sat_halo, int mother_halo, int ngal)
{
  double coulomb, mergtime, SatelliteMass, SatelliteRadius;

  if(sat_halo == mother_halo) 
  {
    printf("\t\tSnapNum, Type, IDs, sat radius:\t%i\t%i\t%i\t%i\t--- sat/cent have the same ID\n", 
      Gal[ngal].SnapNum, Gal[ngal].Type, sat_halo, mother_halo);
    return -1.0;
  }
  
  coulomb = log(Halo[mother_halo].Len / ((double) Halo[sat_halo].Len) + 1);

  SatelliteMass = get_virial_mass(sat_halo) + Gal[ngal].StellarMass + Gal[ngal].ColdGas;
  SatelliteRadius = get_virial_radius(mother_halo);

  if(SatelliteMass > 0.0 && coulomb > 0.0)
    mergtime = 2.0 *
    1.17 * SatelliteRadius * SatelliteRadius * get_virial_velocity(mother_halo) / (coulomb * G * SatelliteMass);
  else
    mergtime = -1.0;
  
  return mergtime;

}



void deal_with_galaxy_merger(int p, int merger_centralgal, int centralgal, double time, double dt, int halonr, int step)
{
  double mi, ma, mass_ratio;

  // calculate mass ratio of merging galaxies 
  if(Gal[p].StellarMass + Gal[p].ColdGas <
    Gal[merger_centralgal].StellarMass + Gal[merger_centralgal].ColdGas)
  {
    mi = Gal[p].StellarMass + Gal[p].ColdGas;
    ma = Gal[merger_centralgal].StellarMass + Gal[merger_centralgal].ColdGas;
  }
  else
  {
    mi = Gal[merger_centralgal].StellarMass + Gal[merger_centralgal].ColdGas;
    ma = Gal[p].StellarMass + Gal[p].ColdGas;
  }

  if(ma > 0)
    mass_ratio = mi / ma;
  else
    mass_ratio = 1.0;

  add_galaxies_together(merger_centralgal, p);

  // grow black hole through accretion from cold disk during mergers, a la Kauffmann & Haehnelt (2000) 
  if(AGNrecipeOn)
    grow_black_hole(merger_centralgal, mass_ratio);
  
  // starburst recipe similar to Somerville et al. 2001
  collisional_starburst_recipe(mass_ratio, merger_centralgal, centralgal, time, dt, halonr, 0, step);

  if(mass_ratio > 0.1)
		Gal[merger_centralgal].TimeSinceMinorMerger = time;

  if(mass_ratio > ThreshMajorMerger)
  {
    make_bulge_from_burst(merger_centralgal);
    Gal[merger_centralgal].TimeSinceMajorMerger = time;
    Gal[p].mergeType = 2;  // mark as major merger
  }
  else
  {
    Gal[p].mergeType = 1;  // mark as minor merger
  }

}



void grow_black_hole(int merger_centralgal, double mass_ratio)
{
  double BHaccrete, metallicity;

  if(Gal[merger_centralgal].ColdGas > 0.0)
  {
    BHaccrete = BlackHoleGrowthRate * mass_ratio / 
      (1.0 + pow(280.0 / Gal[merger_centralgal].Vvir, 2.0)) * Gal[merger_centralgal].ColdGas;

    // cannot accrete more gas than is available! 
    if(BHaccrete > Gal[merger_centralgal].ColdGas)
      BHaccrete = Gal[merger_centralgal].ColdGas;

    metallicity = get_metallicity(Gal[merger_centralgal].ColdGas, Gal[merger_centralgal].MetalsColdGas);
    Gal[merger_centralgal].BlackHoleMass += BHaccrete;
    Gal[merger_centralgal].ColdGas -= BHaccrete;
    Gal[merger_centralgal].MetalsColdGas -= metallicity * BHaccrete;

    Gal[merger_centralgal].QuasarModeBHaccretionMass += BHaccrete;

    quasar_mode_wind(merger_centralgal, BHaccrete);
  }
}



void quasar_mode_wind(int gal, float BHaccrete)
{
  float quasar_energy, cold_gas_energy, hot_gas_energy;
  
  // work out total energies in quasar wind (eta*m*c^2), cold and hot gas (1/2*m*Vvir^2)
  quasar_energy = QuasarModeEfficiency * 0.1 * BHaccrete * (C / UnitVelocity_in_cm_per_s) * (C / UnitVelocity_in_cm_per_s);
  cold_gas_energy = 0.5 * Gal[gal].ColdGas * Gal[gal].Vvir * Gal[gal].Vvir;
  hot_gas_energy = 0.5 * Gal[gal].HotGas * Gal[gal].Vvir * Gal[gal].Vvir;
   
  // compare quasar wind and cold gas energies and eject cold
  if(quasar_energy > cold_gas_energy)
  {
    Gal[gal].EjectedMass += Gal[gal].ColdGas;
    Gal[gal].MetalsEjectedMass += Gal[gal].MetalsColdGas;
   
    Gal[gal].ColdGas = 0.0;
    Gal[gal].MetalsColdGas = 0.0;
  }
  
  // compare quasar wind and cold+hot gas energies and eject hot
  if(quasar_energy > cold_gas_energy + hot_gas_energy)
  {
    Gal[gal].EjectedMass += Gal[gal].HotGas;
    Gal[gal].MetalsEjectedMass += Gal[gal].MetalsHotGas;
   
    Gal[gal].HotGas = 0.0;
    Gal[gal].MetalsHotGas = 0.0;
  }
}



void add_galaxies_together(int t, int p)
{
  int step;
  
  Gal[t].ColdGas += Gal[p].ColdGas;
  Gal[t].MetalsColdGas += Gal[p].MetalsColdGas;
  
  Gal[t].StellarMass += Gal[p].StellarMass;
  Gal[t].MetalsStellarMass += Gal[p].MetalsStellarMass;

  Gal[t].HotGas += Gal[p].HotGas;
  Gal[t].MetalsHotGas += Gal[p].MetalsHotGas;
  
  Gal[t].EjectedMass += Gal[p].EjectedMass;
  Gal[t].MetalsEjectedMass += Gal[p].MetalsEjectedMass;
  
  Gal[t].ICS += Gal[p].ICS;
  Gal[t].MetalsICS += Gal[p].MetalsICS;

  Gal[t].BlackHoleMass += Gal[p].BlackHoleMass;

  // add merger to bulge
	Gal[t].BulgeMass += Gal[p].StellarMass;
	Gal[t].MetalsBulgeMass += Gal[p].MetalsStellarMass;		

  for(step = 0; step < STEPS; step++)
  {
    Gal[t].SfrBulge[step] += Gal[p].SfrDisk[step] + Gal[p].SfrBulge[step];
    Gal[t].SfrBulgeColdGas[step] += Gal[p].SfrDiskColdGas[step] + Gal[p].SfrBulgeColdGas[step];
    Gal[t].SfrBulgeColdGasMetals[step] += Gal[p].SfrDiskColdGasMetals[step] + Gal[p].SfrBulgeColdGasMetals[step];
  }
}



void make_bulge_from_burst(int p)
{
  int step;
  
  // generate bulge 
  Gal[p].BulgeMass = Gal[p].StellarMass;
  Gal[p].MetalsBulgeMass = Gal[p].MetalsStellarMass;

  // update the star formation rate 
  for(step = 0; step < STEPS; step++)
  {
    Gal[p].SfrBulge[step] += Gal[p].SfrDisk[step];
    Gal[p].SfrBulgeColdGas[step] += Gal[p].SfrDiskColdGas[step];
    Gal[p].SfrBulgeColdGasMetals[step] += Gal[p].SfrDiskColdGasMetals[step];
    Gal[p].SfrDisk[step] = 0.0;
    Gal[p].SfrDiskColdGas[step] = 0.0;
    Gal[p].SfrDiskColdGasMetals[step] = 0.0;
  }
}



void collisional_starburst_recipe(double mass_ratio, int merger_centralgal, int centralgal, double time, double dt, int halonr, int mode, int step)
{
  double stars, reheated_mass, ejected_mass, fac, metallicity, eburst;
  double FracZleaveDiskVal;

  // This is the major and minor merger starburst recipe of Somerville et al. 2001. 
  // The coefficients in eburst are taken from TJ Cox's PhD thesis and should be more accurate then previous. 

  // the bursting fraction 
  if(mode == 1)
    eburst = mass_ratio;
  else
    eburst = 0.56 * pow(mass_ratio, 0.7);

  stars = eburst * Gal[merger_centralgal].ColdGas;
  if(stars < 0.0)
    stars = 0.0;

  // this bursting results in SN feedback on the cold/hot gas 
  if(SupernovaRecipeOn == 1)
    reheated_mass = FeedbackReheatingEpsilon * stars;
  else
    reheated_mass = 0.0;

	assert(reheated_mass >= 0.0);

  // can't use more cold gas than is available! so balance SF and feedback 
  if((stars + reheated_mass) > Gal[merger_centralgal].ColdGas)
  {
    fac = Gal[merger_centralgal].ColdGas / (stars + reheated_mass);
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

  // starbursts add to the bulge
  Gal[merger_centralgal].SfrBulge[step] += stars / dt;
  Gal[merger_centralgal].SfrBulgeColdGas[step] += Gal[merger_centralgal].ColdGas;
  Gal[merger_centralgal].SfrBulgeColdGasMetals[step] += Gal[merger_centralgal].MetalsColdGas;

  metallicity = get_metallicity(Gal[merger_centralgal].ColdGas, Gal[merger_centralgal].MetalsColdGas);
  update_from_star_formation(merger_centralgal, stars, metallicity);

  Gal[merger_centralgal].BulgeMass += (1 - RecycleFraction) * stars;
  Gal[merger_centralgal].MetalsBulgeMass += metallicity * (1 - RecycleFraction) * stars;

  // recompute the metallicity of the cold phase
  metallicity = get_metallicity(Gal[merger_centralgal].ColdGas, Gal[merger_centralgal].MetalsColdGas);

  // update from feedback 
  update_from_feedback(merger_centralgal, centralgal, reheated_mass, ejected_mass, metallicity);

  // check for disk instability
  if(DiskInstabilityOn && mode == 0)
    if(mass_ratio < ThreshMajorMerger)
    check_disk_instability(merger_centralgal, centralgal, halonr, time, dt, step);

  // formation of new metals - instantaneous recycling approximation - only SNII 
  if(Gal[merger_centralgal].ColdGas > 1e-8 && mass_ratio < ThreshMajorMerger)
  {
    FracZleaveDiskVal = FracZleaveDisk * exp(-1.0 * Gal[centralgal].Mvir / 30.0);  // Krumholz & Dekel 2011 Eq. 22
    Gal[merger_centralgal].MetalsColdGas += Yield * (1.0 - FracZleaveDiskVal) * stars;
    Gal[centralgal].MetalsHotGas += Yield * FracZleaveDiskVal * stars;
    // Gal[centralgal].MetalsEjectedMass += Yield * FracZleaveDiskVal * stars;
  }
  else
    Gal[centralgal].MetalsHotGas += Yield * stars;
    // Gal[centralgal].MetalsEjectedMass += Yield * stars;
}



void disrupt_satellite_to_ICS(int centralgal, int gal)
{  
  Gal[centralgal].HotGas += Gal[gal].ColdGas + Gal[gal].HotGas;
  Gal[centralgal].MetalsHotGas += Gal[gal].MetalsColdGas + Gal[gal].MetalsHotGas;
  
  Gal[centralgal].EjectedMass += Gal[gal].EjectedMass;
  Gal[centralgal].MetalsEjectedMass += Gal[gal].MetalsEjectedMass;
  
  Gal[centralgal].ICS += Gal[gal].ICS;
  Gal[centralgal].MetalsICS += Gal[gal].MetalsICS;

  Gal[centralgal].ICS += Gal[gal].StellarMass;
  Gal[centralgal].MetalsICS += Gal[gal].MetalsStellarMass;
  
  // what should we do with the disrupted satellite BH?
  
  Gal[gal].mergeType = 4;  // mark as disruption to the ICS
  

}




