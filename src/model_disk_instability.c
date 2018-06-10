#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"



void check_disk_instability(const int p, const int centralgal, const int halonr, const double time, const double dt, const int step, struct GALAXY *galaxies)
{
    // Here we calculate the stability of the stellar and gaseous disk as discussed in Mo, Mao & White (1998).
    // For unstable stars and gas, we transfer the required ammount to the bulge to make the disk stable again

    // Disk mass has to be > 0.0
    const double diskmass = galaxies[p].ColdGas + (galaxies[p].StellarMass - galaxies[p].BulgeMass);
    if(diskmass > 0.0) {
        // calculate critical disk mass
        double Mcrit = galaxies[p].Vmax * galaxies[p].Vmax * (3.0 * galaxies[p].DiskScaleRadius) / run_params.G;
        if(Mcrit > diskmass) {
            Mcrit = diskmass;
        }
    
        // use disk mass here
        const double gas_fraction   = galaxies[p].ColdGas / diskmass;
        const double unstable_gas   = gas_fraction * (diskmass - Mcrit);
        const double star_fraction  = 1.0 - gas_fraction;
        const double unstable_stars = star_fraction * (diskmass - Mcrit);

        // add excess stars to the bulge
        if(unstable_stars > 0.0) {
            // Use disk metallicity here
            const double metallicity = get_metallicity(galaxies[p].StellarMass - galaxies[p].BulgeMass, galaxies[p].MetalsStellarMass - galaxies[p].MetalsBulgeMass);
          
            galaxies[p].BulgeMass += unstable_stars;
            galaxies[p].MetalsBulgeMass += metallicity * unstable_stars;
          
            // Need to fix this. Excluded for now.
            // galaxies[p].mergeType = 3;  // mark as disk instability partial mass transfer
            // galaxies[p].mergeIntoID = NumGals + p - 1;      
          
            if (galaxies[p].BulgeMass / galaxies[p].StellarMass > 1.0001 || galaxies[p].MetalsBulgeMass / galaxies[p].MetalsStellarMass > 1.0001) {
                printf("Instability: Mbulge > Mtot (stars or metals)\n");
                // ABORT(0);
            }
        }

        // burst excess gas and feed black hole (really need a dedicated model for bursts and BH growth here)
        if(unstable_gas > 0.0) {
            if(unstable_gas/galaxies[p].ColdGas > 1.0001) {
                printf("unstable_gas > galaxies[p].ColdGas\t%e\t%e\n", unstable_gas, galaxies[p].ColdGas);
                // ABORT(0);
            }
          
            const double unstable_gas_fraction = unstable_gas / galaxies[p].ColdGas;
            if(run_params.AGNrecipeOn > 0) {
                grow_black_hole(p, unstable_gas_fraction, galaxies);
            }
          
            collisional_starburst_recipe(unstable_gas_fraction, p, centralgal, time, dt, halonr, 1, step, galaxies);
        }
    }
}
