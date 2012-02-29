#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



void check_disk_instability(int p, int centralgal, int halonr, double time, double dt)
{
  double Mcrit, gas_fraction, unstable_gas, unstable_gas_fraction, unstable_stars, diskmass, metallicity;

  // Here we calculate the stability of the stellar and gaseous disk as discussed in Mo, Mao & White (1998).  
  // For unstable stars and gas, we transfer the required ammount to the bulge to make the disk stable again 

  if(Gal[p].ColdGas + Gal[p].StellarMass > 0.0)
  {

    // check stellar+gas disk 
    Mcrit = Gal[p].Vmax * Gal[p].Vmax * (3.0 * Gal[p].DiskScaleRadius) / G;
    diskmass = (Gal[p].ColdGas + Gal[p].StellarMass) - Gal[p].BulgeMass;
    gas_fraction = Gal[p].ColdGas / (Gal[p].ColdGas + Gal[p].StellarMass);

    unstable_gas = gas_fraction * (diskmass - Mcrit);
    unstable_stars = (1.0 - gas_fraction) * (diskmass - Mcrit);

    // add excess stars to the bulge 
    if(unstable_stars > 0.0)
    {
      metallicity = get_metallicity(Gal[p].StellarMass, Gal[p].MetalsStellarMass);
      Gal[p].BulgeMass += unstable_stars;
      Gal[p].MetalsBulgeMass += metallicity * unstable_stars;
    }
    
    // burst excess gas and feed black hole
    if(unstable_gas > 0.0)
    {
      unstable_gas_fraction = unstable_gas / Gal[p].ColdGas;
      if(AGNrecipeOn > 0)
        grow_black_hole(p, unstable_gas_fraction);

      collisional_starburst_recipe(unstable_gas_fraction, p, centralgal, time, dt, halonr, 1);
    }

  }

  // // check stellar disk 
  // Mcrit = Gal[p].Vmax * Gal[p].Vmax * (3.0 * Gal[p].DiskScaleRadius) / G;
  // diskmass = Gal[p].StellarMass - Gal[p].BulgeMass;
  // stars = diskmass - Mcrit;
  // 
  // // add excess stars to the bulge 
  // if(stars > 0.0)
  // {
  //   metallicity = get_metallicity(Gal[p].StellarMass, Gal[p].MetalsStellarMass);
  //   Gal[p].BulgeMass += stars;
  //   Gal[p].MetalsBulgeMass += metallicity * stars;
  // 
  //   fraction = stars / diskmass;
  //   if(AGNrecipeOn > 0)
  //     grow_black_hole(p, fraction);
  // 
  //   collisional_starburst_recipe(fraction, p, centralgal, time, dt, halonr, 1);
  // }

}
