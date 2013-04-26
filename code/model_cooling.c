#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



double cooling_recipe(int gal, double dt)
{
  double tcool, x, logZ, lambda, rcool, rho_rcool, rho0, temp, coolingGas;

  if(Gal[gal].HotGas > 1.0e-6)
  {
    tcool = Gal[gal].Rvir / Gal[gal].Vvir * 10.0;
    temp = 35.9 * Gal[gal].Vvir * Gal[gal].Vvir;         // in Kelvin 

    if(Gal[gal].MetalsHotGas > 0)
      logZ = log10(Gal[gal].MetalsHotGas / Gal[gal].HotGas);
    else
      logZ = -10.0;

    lambda = get_metaldependent_cooling_rate(log10(temp), logZ);
    x = PROTONMASS * BOLTZMANN * temp / lambda;        // now this has units sec g/cm^3  
    x /= (UnitDensity_in_cgs * UnitTime_in_s);         // now in internal units 
    rho_rcool = x / (0.28086 * tcool);

    // an isothermal density profile for the hot gas is assumed here 
    rho0 = Gal[gal].HotGas / (4 * M_PI * Gal[gal].Rvir);
    rcool = sqrt(rho0 / rho_rcool);

    if(rcool > Gal[gal].Rvir)
      // infall dominated regime 
      coolingGas = Gal[gal].HotGas / (Gal[gal].Rvir / Gal[gal].Vvir) * dt; 
    else
      // hot phase regime 
      coolingGas = (Gal[gal].HotGas / Gal[gal].Rvir) * (rcool / tcool) * dt;

    if(coolingGas > Gal[gal].HotGas)
      coolingGas = Gal[gal].HotGas;
    else if(coolingGas < 0.0)
      coolingGas = 0.0;

    if (coolingGas > 0.0)
      Gal[gal].Cooling += 0.5 * coolingGas * Gal[gal].Vvir * Gal[gal].Vvir;

    if(AGNrecipeOn > 0 && Gal[gal].Type == 0)
      coolingGas -= do_AGN_heating(coolingGas, gal, dt, x, rcool);

    if(coolingGas < 0.0)
      coolingGas = 0.0;
  }
  else
    coolingGas = 0.0;

  return coolingGas;

}



double do_AGN_heating(double coolingGas, int centralgal, double dt, double x, double rcool)
{
  double AGNrate, EDDrate, AGNaccreted, AGNcoeff, AGNheating, metallicity;

  if(Gal[centralgal].HotGas > 0.0)
  {

    if(AGNrecipeOn == 2)
    {
      // Bondi-Hoyle accretion recipe -- efficiency = 0.3 
      AGNrate = (2.5 * M_PI * G) * (0.375 * 0.6 * x) * Gal[centralgal].BlackHoleMass * 0.3;
    }
    else if(AGNrecipeOn == 3)
    {
      // Cold cloud accretion: trigger: rBH > 1.0e-4 Rsonic, and accretion rate = 0.01% cooling rate 
      if(Gal[centralgal].BlackHoleMass > 0.0001 * Gal[centralgal].Mvir * pow(rcool/Gal[centralgal].Rvir, 3.0))
        AGNrate = 0.0001 * coolingGas / dt;
      else
        AGNrate = 0.0;
    }
    else
    {
      // empirical (standard) accretion recipe 
      AGNrate = RadioModeEfficiency / (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS)
        * (Gal[centralgal].BlackHoleMass / 0.01) * pow(Gal[centralgal].Vvir / 200.0, 3.0)
        * ((Gal[centralgal].HotGas / Gal[centralgal].Mvir) / 0.1);
    }
    
    // Eddington rate 
    EDDrate = 1.3e48 * Gal[centralgal].BlackHoleMass / (UnitEnergy_in_cgs / UnitTime_in_s) / 9e10;

    // accretion onto BH is always limited by the Eddington rate 
    if(AGNrate > EDDrate)
      AGNrate = EDDrate;

    // accreted mass onto black hole 
    AGNaccreted = AGNrate * dt;

    // cannot accrete more mass than is available! 
    if(AGNaccreted > Gal[centralgal].HotGas)
      AGNaccreted = Gal[centralgal].HotGas;

    // coefficient to heat the cooling gas back to the virial temperature of the halo 
    // 1.34e5 = sqrt(2*eta*c^2), eta=0.1 (standard efficiency) and c in km/s 
    AGNcoeff = (1.34e5 / Gal[centralgal].Vvir) * (1.34e5 / Gal[centralgal].Vvir);

    // cooling mass that can be suppresed from AGN heating 
    AGNheating = AGNcoeff * AGNaccreted;

    // limit heating to cooling rate 
    if(AGNheating > coolingGas)
    {
      AGNaccreted = coolingGas / AGNcoeff;
      AGNheating = coolingGas;
    }

    // accreted mass onto black hole 
    metallicity = get_metallicity(Gal[centralgal].HotGas, Gal[centralgal].MetalsHotGas);
    Gal[centralgal].BlackHoleMass += AGNaccreted;
    Gal[centralgal].HotGas -= AGNaccreted;
    Gal[centralgal].MetalsHotGas -= metallicity * AGNaccreted;
  }
  else
    AGNheating = 0.0;

  if (AGNheating > 0.0)
    Gal[centralgal].Heating += 0.5 * AGNheating * Gal[centralgal].Vvir * Gal[centralgal].Vvir;

  return AGNheating;

}



void cool_gas_onto_galaxy(int centralgal, double coolingGas)
{
  double metallicity;

  // add the fraction 1/STEPS of the total cooling gas to the cold disk 
  if(coolingGas > 0.0)
  {
    if(coolingGas < Gal[centralgal].HotGas)
    {
      metallicity = get_metallicity(Gal[centralgal].HotGas, Gal[centralgal].MetalsHotGas);
      Gal[centralgal].ColdGas += coolingGas;
      Gal[centralgal].MetalsColdGas += metallicity * coolingGas;
      Gal[centralgal].HotGas -= coolingGas;
      Gal[centralgal].MetalsHotGas -= metallicity * coolingGas;
    }
    else
    {
      Gal[centralgal].ColdGas += Gal[centralgal].HotGas;
      Gal[centralgal].MetalsColdGas += Gal[centralgal].MetalsHotGas;
      Gal[centralgal].HotGas = 0.0;
      Gal[centralgal].MetalsHotGas = 0.0;
    }
  }

}


