#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"



double cooling_recipe(const int gal, const double dt, struct GALAXY *galaxies)
{
    double coolingGas;

    if(galaxies[gal].HotGas > 0.0 && galaxies[gal].Vvir > 0.0) {
        const double tcool = galaxies[gal].Rvir / galaxies[gal].Vvir;
        const double temp = 35.9 * galaxies[gal].Vvir * galaxies[gal].Vvir;         // in Kelvin 

        double logZ = -10.0;
        if(galaxies[gal].MetalsHotGas > 0) {
            logZ = log10(galaxies[gal].MetalsHotGas / galaxies[gal].HotGas);
        }

        double lambda = get_metaldependent_cooling_rate(log10(temp), logZ);
        double x = PROTONMASS * BOLTZMANN * temp / lambda;        // now this has units sec g/cm^3  
        x /= (UnitDensity_in_cgs * UnitTime_in_s);         // now in internal units 
        const double rho_rcool = x / tcool * 0.885;  // 0.885 = 3/2 * mu, mu=0.59 for a fully ionized gas

        // an isothermal density profile for the hot gas is assumed here 
        const double rho0 = galaxies[gal].HotGas / (4 * M_PI * galaxies[gal].Rvir);
        const double rcool = sqrt(rho0 / rho_rcool);

        coolingGas = 0.0;
        if(rcool > galaxies[gal].Rvir) {
            // "cold accretion" regime 
            coolingGas = galaxies[gal].HotGas / (galaxies[gal].Rvir / galaxies[gal].Vvir) * dt; 
        } else {
            // "hot halo cooling" regime 
            coolingGas = (galaxies[gal].HotGas / galaxies[gal].Rvir) * (rcool / (2.0 * tcool)) * dt;
        }

        if(coolingGas > galaxies[gal].HotGas) {
            coolingGas = galaxies[gal].HotGas;
        } else { 
			if(coolingGas < 0.0) coolingGas = 0.0;
        }

		// at this point we have calculated the maximal cooling rate
		// if AGNrecipeOn we now reduce it in line with past heating before proceeding

		if(AGNrecipeOn > 0 && coolingGas > 0.0) {
			coolingGas = do_AGN_heating(coolingGas, gal, dt, x, rcool, galaxies);
        }
	
		if (coolingGas > 0.0) {
			galaxies[gal].Cooling += 0.5 * coolingGas * galaxies[gal].Vvir * galaxies[gal].Vvir;
        }
	} else {
		coolingGas = 0.0;
    }

	assert(coolingGas >= 0.0);
    return coolingGas;

}



double do_AGN_heating(double coolingGas, const int centralgal, const double dt, const double x, const double rcool, struct GALAXY *galaxies)
{
    double AGNrate, EDDrate, AGNaccreted, AGNcoeff, AGNheating, metallicity, r_heat_new;

	// first update the cooling rate based on the past AGN heating
	if(galaxies[centralgal].r_heat < rcool) {
		coolingGas = (1.0 - galaxies[centralgal].r_heat / rcool) * coolingGas;
    } else {
		coolingGas = 0.0;
    }
	
	assert(coolingGas >= 0.0);

	// now calculate the new heating rate
    if(galaxies[centralgal].HotGas > 0.0) {
        if(AGNrecipeOn == 2) {
            // Bondi-Hoyle accretion recipe
            AGNrate = (2.5 * M_PI * G) * (0.375 * 0.6 * x) * galaxies[centralgal].BlackHoleMass * RadioModeEfficiency;
        } else if(AGNrecipeOn == 3) {
            // Cold cloud accretion: trigger: rBH > 1.0e-4 Rsonic, and accretion rate = 0.01% cooling rate 
            if(galaxies[centralgal].BlackHoleMass > 0.0001 * galaxies[centralgal].Mvir * pow(rcool/galaxies[centralgal].Rvir, 3.0)) {
                AGNrate = 0.0001 * coolingGas / dt;
            } else {
                AGNrate = 0.0;
            }
        } else {
            // empirical (standard) accretion recipe 
            if(galaxies[centralgal].Mvir > 0.0) {
                AGNrate = RadioModeEfficiency / (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS)
                    * (galaxies[centralgal].BlackHoleMass / 0.01) * pow(galaxies[centralgal].Vvir / 200.0, 3.0)
                    * ((galaxies[centralgal].HotGas / galaxies[centralgal].Mvir) / 0.1);
            } else {
                AGNrate = RadioModeEfficiency / (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS)
                    * (galaxies[centralgal].BlackHoleMass / 0.01) * pow(galaxies[centralgal].Vvir / 200.0, 3.0);
            }
        }
        
        // Eddington rate 
        EDDrate = (1.3e38 * galaxies[centralgal].BlackHoleMass * 1e10 / Hubble_h) / (UnitEnergy_in_cgs / UnitTime_in_s) / (0.1 * 9e10);
        
        // accretion onto BH is always limited by the Eddington rate 
        if(AGNrate > EDDrate) {
            AGNrate = EDDrate;
        }

        // accreted mass onto black hole 
        AGNaccreted = AGNrate * dt;
        
        // cannot accrete more mass than is available! 
        if(AGNaccreted > galaxies[centralgal].HotGas) {
            AGNaccreted = galaxies[centralgal].HotGas;
        }

        // coefficient to heat the cooling gas back to the virial temperature of the halo 
        // 1.34e5 = sqrt(2*eta*c^2), eta=0.1 (standard efficiency) and c in km/s 
        AGNcoeff = (1.34e5 / galaxies[centralgal].Vvir) * (1.34e5 / galaxies[centralgal].Vvir);
        
        // cooling mass that can be suppresed from AGN heating 
        AGNheating = AGNcoeff * AGNaccreted;
        
        /// the above is the maximal heating rate. we now limit it to the current cooling rate
        if(AGNheating > coolingGas) {
            AGNaccreted = coolingGas / AGNcoeff;
            AGNheating = coolingGas;
        }

        // accreted mass onto black hole 
        metallicity = get_metallicity(galaxies[centralgal].HotGas, galaxies[centralgal].MetalsHotGas);
        galaxies[centralgal].BlackHoleMass += AGNaccreted;
        galaxies[centralgal].HotGas -= AGNaccreted;
        galaxies[centralgal].MetalsHotGas -= metallicity * AGNaccreted;
        
        // update the heating radius as needed
        if(galaxies[centralgal].r_heat < rcool && coolingGas > 0.0) {
            r_heat_new = (AGNheating / coolingGas) * rcool;
            if(r_heat_new > galaxies[centralgal].r_heat) {
                galaxies[centralgal].r_heat = r_heat_new;
            }
        }
		
        if (AGNheating > 0.0) {
            galaxies[centralgal].Heating += 0.5 * AGNheating * galaxies[centralgal].Vvir * galaxies[centralgal].Vvir;
        }
    }

    return coolingGas;
}



void cool_gas_onto_galaxy(const int centralgal, const double coolingGas, struct GALAXY *galaxies)
{
    // add the fraction 1/STEPS of the total cooling gas to the cold disk 
    if(coolingGas > 0.0) {
        if(coolingGas < galaxies[centralgal].HotGas) {
            const double metallicity = get_metallicity(galaxies[centralgal].HotGas, galaxies[centralgal].MetalsHotGas);
            galaxies[centralgal].ColdGas += coolingGas;
            galaxies[centralgal].MetalsColdGas += metallicity * coolingGas;
            galaxies[centralgal].HotGas -= coolingGas;
            galaxies[centralgal].MetalsHotGas -= metallicity * coolingGas;
        } else {
            galaxies[centralgal].ColdGas += galaxies[centralgal].HotGas;
            galaxies[centralgal].MetalsColdGas += galaxies[centralgal].MetalsHotGas;
            galaxies[centralgal].HotGas = 0.0;
            galaxies[centralgal].MetalsHotGas = 0.0;
        }
    }
}


