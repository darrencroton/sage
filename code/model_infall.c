#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



double infall_recipe(int centralgal, int ngal, double Zcurr)
{
  int i;
  double tot_stellarMass, tot_BHMass, tot_coldMass, tot_hotMass, tot_ejected, tot_ICS;
	double tot_hotMetals, tot_ejectedMetals, tot_ICSMetals;
	double tot_satBaryons, newSatBaryons;
  double infallingMass, reionization_modifier;

  // need to add up all the baryonic mass asociated with the full halo 
  tot_stellarMass = tot_coldMass = tot_hotMass = tot_hotMetals = tot_ejected = tot_BHMass = tot_ejectedMetals = tot_ICS = tot_ICSMetals = tot_satBaryons = 0.0;

	// loop over all galaxies in the FoF-halo 
  for(i = 0; i < ngal; i++)
  {
    tot_stellarMass += Gal[i].StellarMass;
    tot_BHMass += Gal[i].BlackHoleMass;
    tot_coldMass += Gal[i].ColdGas;
    tot_hotMass += Gal[i].HotGas;
    tot_hotMetals += Gal[i].MetalsHotGas;
    tot_ejected += Gal[i].EjectedMass;
    tot_ejectedMetals += Gal[i].MetalsEjectedMass;
    tot_ICS += Gal[i].ICS;
    tot_ICSMetals += Gal[i].MetalsICS;

		// record the current baryons in satellites only
    if(i != centralgal)
			tot_satBaryons += Gal[i].StellarMass + Gal[i].BlackHoleMass + Gal[i].ColdGas + Gal[i].HotGas;

    // satellite ejected gas goes to central ejected reservior
    if(i != centralgal)
      Gal[i].EjectedMass = Gal[i].MetalsEjectedMass = 0.0;

    // satellite ICS goes to central ICS
    if(i != centralgal) 
      Gal[i].ICS = Gal[i].MetalsICS = 0.0; 
  }

	// the existing baryons that have fallen in with substructure since the last timestep
	newSatBaryons = tot_satBaryons - Gal[centralgal].TotalSatelliteBaryons;

  // include reionization if necessary 
  if(ReionizationOn)
    reionization_modifier = do_reionization(centralgal, Zcurr);
  else
    reionization_modifier = 1.0;

  infallingMass =
    reionization_modifier * BaryonFrac * Gal[centralgal].Mvir - (tot_stellarMass + tot_coldMass + tot_hotMass + tot_ejected + tot_BHMass + tot_ICS);
    // reionization_modifier * BaryonFrac * Gal[centralgal].deltaMvir - newSatBaryons;

  // the central galaxy keeps all the ejected mass
  Gal[centralgal].EjectedMass = tot_ejected;
  Gal[centralgal].MetalsEjectedMass = tot_ejectedMetals;

  if(Gal[centralgal].MetalsEjectedMass > Gal[centralgal].EjectedMass)
    Gal[centralgal].MetalsEjectedMass = Gal[centralgal].EjectedMass;
  if(Gal[centralgal].EjectedMass < 0.0)
    Gal[centralgal].EjectedMass = Gal[centralgal].MetalsEjectedMass = 0.0;
  if(Gal[centralgal].MetalsEjectedMass < 0.0)
    Gal[centralgal].MetalsEjectedMass = 0.0;

  // the central galaxy keeps all the ICS (mostly for numerical convenience)
  Gal[centralgal].ICS = tot_ICS;
  Gal[centralgal].MetalsICS = tot_ICSMetals;

  if(Gal[centralgal].MetalsICS > Gal[centralgal].ICS)
    Gal[centralgal].MetalsICS = Gal[centralgal].ICS;
  if(Gal[centralgal].ICS < 0.0)
    Gal[centralgal].ICS = Gal[centralgal].MetalsICS = 0.0;
  if(Gal[centralgal].MetalsICS < 0.0)
    Gal[centralgal].MetalsICS = 0.0;

  return infallingMass;
}



void strip_from_satellite(int halonr, int centralgal, int gal)
{
  double reionization_modifier, strippedGas, strippedGasMetals, metallicity;
  
  if(ReionizationOn)
    reionization_modifier = do_reionization(gal, ZZ[Halo[halonr].SnapNum]);
  else
    reionization_modifier = 1.0;
  
  strippedGas = -1.0 *
    (reionization_modifier * BaryonFrac * Gal[gal].Mvir - (Gal[gal].StellarMass + Gal[gal].ColdGas + Gal[gal].HotGas + Gal[gal].EjectedMass + Gal[gal].BlackHoleMass + Gal[gal].ICS) ) / STEPS;
    // ( reionization_modifier * BaryonFrac * Gal[gal].deltaMvir ) / STEPS;

  if(strippedGas > 0.0)
  {
    metallicity = get_metallicity(Gal[gal].HotGas, Gal[gal].MetalsHotGas);
    strippedGasMetals = strippedGas * metallicity;
  
    if(strippedGas > Gal[gal].HotGas) strippedGas = Gal[gal].HotGas;
    if(strippedGasMetals > Gal[gal].MetalsHotGas) strippedGasMetals = Gal[gal].MetalsHotGas;

    Gal[gal].HotGas -= strippedGas;
    Gal[gal].MetalsHotGas -= strippedGasMetals;

    Gal[centralgal].HotGas += strippedGas;
    Gal[centralgal].MetalsHotGas += strippedGas * metallicity;
  }
  
}



double do_reionization(int gal, double Zcurr)
{
  double alpha, a, f_of_a, a_on_a0, a_on_ar, Mfiltering, Mjeans, Mchar, mass_to_use, modifier;
  double Tvir, Vchar, omegaZ, xZ, deltacritZ, HubbleZ;

  // we employ the reionization recipie described in Gnedin (2000), however use the fitting 
  // formulas given by Kravtsov et al (2004) Appendix B 

  // here are two parameters that Kravtsov et al keep fixed, alpha gives the best fit to the Gnedin data 
  alpha = 6.0;
  Tvir = 1e4;

  // calculate the filtering mass 
  a = 1.0 / (1.0 + Zcurr);
  a_on_a0 = a / a0;
  a_on_ar = a / ar;

  if(a <= a0)
    f_of_a = 3.0 * a / ((2.0 + alpha) * (5.0 + 2.0 * alpha)) * pow(a_on_a0, alpha);
  else if((a > a0) && (a < ar))
    f_of_a =
    (3.0 / a) * a0 * a0 * (1.0 / (2.0 + alpha) - 2.0 * pow(a_on_a0, -0.5) / (5.0 + 2.0 * alpha)) +
    a * a / 10.0 - (a0 * a0 / 10.0) * (5.0 - 4.0 * pow(a_on_a0, -0.5));
  else
    f_of_a =
    (3.0 / a) * (a0 * a0 * (1.0 / (2.0 + alpha) - 2.0 * pow(a_on_a0, -0.5) / (5.0 + 2.0 * alpha)) +
    (ar * ar / 10.0) * (5.0 - 4.0 * pow(a_on_ar, -0.5)) - (a0 * a0 / 10.0) * (5.0 -
    4.0 *
    pow(a_on_a0,
    -0.5)) +
    a * ar / 3.0 - (ar * ar / 3.0) * (3.0 - 2.0 * pow(a_on_ar, -0.5)));

  // this is in units of 10^10Msun/h, note mu=0.59 and mu^-1.5 = 2.21 
  Mjeans = 25.0 * pow(Omega, -0.5) * 2.21;
  Mfiltering = Mjeans * pow(f_of_a, 1.5);

  // calculate the characteristic mass coresponding to a halo temperature of 10^4K 
  Vchar = sqrt(Tvir / 36.0);
  omegaZ = Omega * (pow(1.0 + Zcurr, 3.0) / (Omega * pow(1.0 + Zcurr, 3.0) + OmegaLambda));
  xZ = omegaZ - 1.0;
  deltacritZ = 18.0 * M_PI * M_PI + 82.0 * xZ - 39.0 * xZ * xZ;
  HubbleZ = Hubble * sqrt(Omega * pow(1.0 + Zcurr, 3.0) + OmegaLambda);

  Mchar = Vchar * Vchar * Vchar / (G * HubbleZ * sqrt(0.5 * deltacritZ));

  // we use the maximum of Mfiltering and Mchar 
  mass_to_use = dmax(Mfiltering, Mchar);
  modifier = 1.0 / pow(1.0 + 0.26 * (mass_to_use / Gal[gal].Mvir), 3.0);

  return modifier;

}



void add_infall_to_hot(int gal, double infallingGas)
{
  float metallicity;

  // if the halo has lost mass, subtract baryons from the ejected mass first, then the hot gas
  if(infallingGas < 0.0 && Gal[gal].EjectedMass > 0.0)
  {  
    metallicity = get_metallicity(Gal[gal].EjectedMass, Gal[gal].MetalsEjectedMass);
    Gal[gal].MetalsEjectedMass += infallingGas*metallicity;
    if(Gal[gal].MetalsEjectedMass < 0.0) Gal[gal].MetalsEjectedMass = 0.0;

    Gal[gal].EjectedMass += infallingGas;
    if(Gal[gal].EjectedMass < 0.0)
    {
      infallingGas = Gal[gal].EjectedMass;
      Gal[gal].EjectedMass = Gal[gal].MetalsEjectedMass = 0.0;
    }
    else
      infallingGas = 0.0;
  }

  // if the halo has lost mass, subtract hot metals mass next, then the hot gas
  if(infallingGas < 0.0 && Gal[gal].MetalsHotGas > 0.0)
  {
    metallicity = get_metallicity(Gal[gal].HotGas, Gal[gal].MetalsHotGas);
    Gal[gal].MetalsHotGas += infallingGas*metallicity;
    if(Gal[gal].MetalsHotGas < 0.0) Gal[gal].MetalsHotGas = 0.0;
  }

  // add (subtract) the ambient (enriched) infalling gas to the central galaxy hot component 
  Gal[gal].HotGas += infallingGas;
  if(Gal[gal].HotGas < 0.0) Gal[gal].HotGas = Gal[gal].MetalsHotGas = 0.0;

}

