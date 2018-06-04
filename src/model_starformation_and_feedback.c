#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"



void starformation_and_feedback(const int p, const int centralgal, const double time, const double dt, const int halonr, const int step, struct GALAXY *galaxies)
{
    double reff, tdyn, strdot, stars, ejected_mass, fac, metallicity;
    double cold_crit;
    double FracZleaveDiskVal;
  
    // Initialise variables
    strdot = 0.0;

    // star formation recipes 
    if(SFprescription == 0) {
        // we take the typical star forming region as 3.0*r_s using the Milky Way as a guide
        reff = 3.0 * galaxies[p].DiskScaleRadius;
        tdyn = reff / galaxies[p].Vvir;
        
        // from Kauffmann (1996) eq7 x piR^2, (Vvir in km/s, reff in Mpc/h) in units of 10^10Msun/h 
        cold_crit = 0.19 * galaxies[p].Vvir * reff;
        if(galaxies[p].ColdGas > cold_crit && tdyn > 0.0) {
            strdot = SfrEfficiency * (galaxies[p].ColdGas - cold_crit) / tdyn;
        } else {
            strdot = 0.0;
        }
    } else {
        printf("No star formation prescription selected!\n");
        ABORT(0);
    }

    stars = strdot * dt;
    if(stars < 0.0) {
        stars = 0.0;
    }

    double reheated_mass = (SupernovaRecipeOn == 1) ? FeedbackReheatingEpsilon * stars: 0.0;

	assert(reheated_mass >= 0.0);

    // cant use more cold gas than is available! so balance SF and feedback 
    if((stars + reheated_mass) > galaxies[p].ColdGas && (stars + reheated_mass) > 0.0) {
        fac = galaxies[p].ColdGas / (stars + reheated_mass);
        stars *= fac;
        reheated_mass *= fac;
    }
    
    // determine ejection
    if(SupernovaRecipeOn == 1) {
        if(galaxies[centralgal].Vvir > 0.0) {
            ejected_mass = 
                (FeedbackEjectionEfficiency * (EtaSNcode * EnergySNcode) / (galaxies[centralgal].Vvir * galaxies[centralgal].Vvir) -
                 FeedbackReheatingEpsilon) * stars;
        } else {
            ejected_mass = 0.0;
        }
		
        if(ejected_mass < 0.0) {
            ejected_mass = 0.0;
        }
    } else {
        ejected_mass = 0.0;
    }

    // update the star formation rate 
    galaxies[p].SfrDisk[step] += stars / dt;
    galaxies[p].SfrDiskColdGas[step] = galaxies[p].ColdGas;
    galaxies[p].SfrDiskColdGasMetals[step] = galaxies[p].MetalsColdGas;

    // update for star formation 
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);
    update_from_star_formation(p, stars, metallicity, galaxies);

    // recompute the metallicity of the cold phase
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);

    // update from SN feedback 
    update_from_feedback(p, centralgal, reheated_mass, ejected_mass, metallicity, galaxies);

    // check for disk instability
    if(DiskInstabilityOn) {
        check_disk_instability(p, centralgal, halonr, time, dt, step, galaxies);
    }

    // formation of new metals - instantaneous recycling approximation - only SNII 
    if(galaxies[p].ColdGas > 1.0e-8) {
        FracZleaveDiskVal = FracZleaveDisk * exp(-1.0 * galaxies[centralgal].Mvir / 30.0);  // Krumholz & Dekel 2011 Eq. 22
        galaxies[p].MetalsColdGas += Yield * (1.0 - FracZleaveDiskVal) * stars;
        galaxies[centralgal].MetalsHotGas += Yield * FracZleaveDiskVal * stars;
        // galaxies[centralgal].MetalsEjectedMass += Yield * FracZleaveDiskVal * stars;
    } else {
        galaxies[centralgal].MetalsHotGas += Yield * stars;
        // galaxies[centralgal].MetalsEjectedMass += Yield * stars;
    }
}



void update_from_star_formation(const int p, const double stars, const double metallicity, struct GALAXY *galaxies)
{
    // update gas and metals from star formation 
    galaxies[p].ColdGas -= (1 - RecycleFraction) * stars;
    galaxies[p].MetalsColdGas -= metallicity * (1 - RecycleFraction) * stars;
    galaxies[p].StellarMass += (1 - RecycleFraction) * stars;
    galaxies[p].MetalsStellarMass += metallicity * (1 - RecycleFraction) * stars;
}



void update_from_feedback(const int p, const int centralgal, const double reheated_mass, double ejected_mass, const double metallicity, struct GALAXY *galaxies)
{
	assert(!(reheated_mass > galaxies[p].ColdGas && reheated_mass > 0.0));

    if(SupernovaRecipeOn == 1) {
        galaxies[p].ColdGas -= reheated_mass;
        galaxies[p].MetalsColdGas -= metallicity * reheated_mass;
        
        galaxies[centralgal].HotGas += reheated_mass;
        galaxies[centralgal].MetalsHotGas += metallicity * reheated_mass;
        
        if(ejected_mass > galaxies[centralgal].HotGas) {
            ejected_mass = galaxies[centralgal].HotGas;
        }
        const double metallicityHot = get_metallicity(galaxies[centralgal].HotGas, galaxies[centralgal].MetalsHotGas);

        galaxies[centralgal].HotGas -= ejected_mass;
        galaxies[centralgal].MetalsHotGas -= metallicityHot * ejected_mass;
        galaxies[centralgal].EjectedMass += ejected_mass;
        galaxies[centralgal].MetalsEjectedMass += metallicityHot * ejected_mass;
        
        galaxies[p].OutflowRate += reheated_mass;    
    }
}


