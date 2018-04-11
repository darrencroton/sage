#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



void check_disk_instability(int p, int centralgal, int halonr, double time, double dt, int step)
{
  double Mcrit, gas_fraction, unstable_gas, unstable_gas_fraction, unstable_stars, diskmass, metallicity;
  double star_fraction;

  // Here we calculate the stability of the stellar and gaseous disk as discussed in Mo, Mao & White (1998).
  // For unstable stars and gas, we transfer the required ammount to the bulge to make the disk stable again

  // Disk mass has to be > 0.0
  diskmass = Gal[p].ColdGas + (Gal[p].StellarMass - Gal[p].BulgeMass);
  if(diskmass > 0.0)
  {
    // calculate critical disk mass
    Mcrit = Gal[p].Vmax * Gal[p].Vmax * (3.0 * Gal[p].DiskScaleRadius) / G;
    if(Mcrit > diskmass)
      Mcrit = diskmass;
    
    // use disk mass here
    gas_fraction   = Gal[p].ColdGas / diskmass;
    unstable_gas   = gas_fraction * (diskmass - Mcrit);
    star_fraction  = 1.0 - gas_fraction;
    unstable_stars = star_fraction * (diskmass - Mcrit);

    // add excess stars to the bulge
    if(unstable_stars > 0.0)
    {
      // Use disk metallicity here
      metallicity = get_metallicity(Gal[p].StellarMass - Gal[p].BulgeMass, Gal[p].MetalsStellarMass - Gal[p].MetalsBulgeMass);

      Gal[p].BulgeMass += unstable_stars;
      Gal[p].MetalsBulgeMass += metallicity * unstable_stars;
      
      // Need to fix this. Excluded for now.
      // Gal[p].mergeType = 3;  // mark as disk instability partial mass transfer
      // Gal[p].mergeIntoID = NumGals + p - 1;      
      
      if (Gal[p].BulgeMass / Gal[p].StellarMass > 1.0001 || Gal[p].MetalsBulgeMass / Gal[p].MetalsStellarMass > 1.0001)
	    {
        printf("Instability: Mbulge > Mtot (stars or metals)\n");
        // ABORT(0);
      }
    }

    // burst excess gas and feed black hole (really need a dedicated model for bursts and BH growth here)
    if(unstable_gas > 0.0)
    {
      if(unstable_gas/Gal[p].ColdGas > 1.0001)
      {
        printf("unstable_gas > Gal[p].ColdGas\t%e\t%e\n", unstable_gas, Gal[p].ColdGas);
        // ABORT(0);
      }

      unstable_gas_fraction = unstable_gas / Gal[p].ColdGas;
      if(AGNrecipeOn > 0)
        grow_black_hole(p, unstable_gas_fraction);
    
      collisional_starburst_recipe(unstable_gas_fraction, p, centralgal, time, dt, halonr, 1, step);
    }

  }

}
