#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



double estimate_merging_time(int sat_halo, int mother_halo, int ngal)
{
  double coulomb, mergtime, SatelliteMass, SatelliteRadius;
  double distx, disty, distz;

  coulomb = log(Halo[mother_halo].Len / ((double) Halo[sat_halo].Len) + 1);

  SatelliteMass = get_virial_mass(sat_halo) + Gal[ngal].StellarMass + Gal[ngal].ColdGas;

  distx = fabs(Halo[mother_halo].Pos[0] - Halo[sat_halo].Pos[0]);
  if(distx > 20.0) distx = BoxSideLength - distx;
  
  disty = fabs(Halo[mother_halo].Pos[1] - Halo[sat_halo].Pos[1]);
  if(disty > 20.0) disty = BoxSideLength - disty;

  distz = fabs(Halo[mother_halo].Pos[2] - Halo[sat_halo].Pos[2]);
  if(distz > 20.0) distz = BoxSideLength - distz;

  SatelliteRadius = sqrt(distx*distx + disty*disty + distz*distz);
  
  if(SatelliteRadius <= 0.0 || SatelliteRadius > 20.0) 
  {
    printf("\t\tSnapNum, Type, IDs, sat radius:\t%i\t%i\t%i\t%i\t%f\t", 
      Gal[ngal].SnapNum, Gal[ngal].Type, sat_halo, mother_halo, SatelliteRadius);
    if(SatelliteRadius > 20.0)
    { 
      printf("||| galaxies not spatially associated\n");
      return 999.0;
    }
    if(sat_halo == mother_halo)
    {
      printf("--- sat/cent have the same ID\n");
      Gal[ngal].AlreadyMerged = 1;
      return -1.0;
    }
    printf("\n");
    ABORT(66);
  }

  // convert to physical length 
  SatelliteRadius /= (1 + ZZ[Halo[sat_halo].SnapNum]);

  if(SatelliteMass > 0.0)
    mergtime = 
    1.17 * SatelliteRadius * SatelliteRadius * get_virial_velocity(mother_halo) / (coulomb * G * SatelliteMass);
  else
    mergtime = -1.0;
  
  return mergtime;

}



void deal_with_galaxy_merger(int p, int merger_centralgal, int centralgal, double time, double dt, int halonr)
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
  collisional_starburst_recipe(mass_ratio, merger_centralgal, centralgal, time, dt, halonr, 0);
  if(mass_ratio > ThreshMajorMerger)
    make_bulge_from_burst(merger_centralgal);

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
  int outputbin;

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

  for(outputbin = 0; outputbin < NOUT; outputbin++)
  {
    Gal[t].Sfr[outputbin] += Gal[p].Sfr[outputbin];
    Gal[t].SfrICS[outputbin] += Gal[p].SfrICS[outputbin];
  }

 // add merger to bulge
  Gal[t].BulgeMass += Gal[p].StellarMass;
  Gal[t].MetalsBulgeMass += Gal[p].MetalsStellarMass;
  for(outputbin = 0; outputbin < NOUT; outputbin++)
    Gal[t].SfrBulge[outputbin] += Gal[p].Sfr[outputbin];
}



void make_bulge_from_burst(int p)
{
  int outputbin;

  // generate bulge 
  Gal[p].BulgeMass = Gal[p].StellarMass;
  Gal[p].MetalsBulgeMass = Gal[p].MetalsStellarMass;

  // update the star formation rate 
  for(outputbin = 0; outputbin < NOUT; outputbin++)
    Gal[p].SfrBulge[outputbin] = Gal[p].Sfr[outputbin];
}



void collisional_starburst_recipe(double mass_ratio, int merger_centralgal, int centralgal, double time, double dt, int halonr, int mode)
{
  double stars, reheated_mass, ejected_mass, fac, metallicity, CentralVvir, eburst;
  double FracZleaveDiskVal;
  int outputbin;

  // This is the major and minor merger starburst recipe of Somerville et al. 2001. 
  // The coefficients in eburst are taken from TJ Cox's PhD thesis and should be more 
  // accurate then previous. 

  CentralVvir = Gal[centralgal].Vvir;

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

  if(reheated_mass < 0.0)
  {
    printf("Something strange here ....\n");
    ABORT(32);
  }

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
    ejected_mass = 
      // 0.5 *  // XXX - New ejection/reincorporation scheme
      (FeedbackEjectionEfficiency * (EtaSNcode * EnergySNcode) / (CentralVvir * CentralVvir) - 
      FeedbackReheatingEpsilon) * stars;
    if(ejected_mass < 0.0)
      ejected_mass = 0.0;
  }
  else
    ejected_mass = 0.0;

  // update the star formation rate 
  for(outputbin = 0; outputbin < NOUT; outputbin++)
  {
    if(Halo[halonr].SnapNum == ListOutputSnaps[outputbin])
    {
      Gal[merger_centralgal].Sfr[outputbin] += stars / (dt * STEPS);
      if(mode == 1) Gal[merger_centralgal].SfrBulge[outputbin] += stars / (dt * STEPS);
      break;
    }
  }

  // update for star formation 
  metallicity = get_metallicity(Gal[merger_centralgal].ColdGas, Gal[merger_centralgal].MetalsColdGas);
  update_from_star_formation(merger_centralgal, stars, metallicity);
  if(mode == 1)
  {
    Gal[merger_centralgal].BulgeMass += (1 - RecycleFraction) * stars;
    Gal[merger_centralgal].MetalsBulgeMass += metallicity * (1 - RecycleFraction) * stars;
  }

  // recompute the metallicity of the cold phase
  metallicity = get_metallicity(Gal[merger_centralgal].ColdGas, Gal[merger_centralgal].MetalsColdGas);

  // update from feedback 
  update_from_feedback(merger_centralgal, centralgal, reheated_mass, ejected_mass, metallicity);

  // check for disk instability
  if(DiskInstabilityOn && mode == 0)
    if(mass_ratio < ThreshMajorMerger)
    check_disk_instability(merger_centralgal, centralgal, halonr, time, dt);

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
  int outputbin;
  
  Gal[centralgal].HotGas += Gal[gal].ColdGas + Gal[gal].HotGas;
  Gal[centralgal].MetalsHotGas += Gal[gal].MetalsColdGas + Gal[gal].MetalsHotGas;
  
  Gal[centralgal].EjectedMass += Gal[gal].EjectedMass;
  Gal[centralgal].MetalsEjectedMass += Gal[gal].MetalsEjectedMass;
  
  Gal[centralgal].ICS += Gal[gal].ICS;
  Gal[centralgal].MetalsICS += Gal[gal].MetalsICS;

  Gal[centralgal].ICS += Gal[gal].StellarMass;
  Gal[centralgal].MetalsICS += Gal[gal].MetalsStellarMass;
  
  for(outputbin = 0; outputbin < NOUT; outputbin++)
  {
    Gal[centralgal].SfrICS[outputbin] += Gal[gal].Sfr[outputbin];
    Gal[centralgal].SfrICS[outputbin] += Gal[gal].SfrICS[outputbin];
  }

  // what should we do with the disrupted satellite BH?

}




