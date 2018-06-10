#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"


double estimate_merging_time(const int sat_halo, const int mother_halo, const int ngal, struct halo_data *halos, struct GALAXY *galaxies)
{
    double mergtime;
    const int MinNumPartSatHalo = 10;
    
    if(sat_halo == mother_halo) {
        printf("\t\tSnapNum, Type, IDs, sat radius:\t%i\t%i\t%i\t%i\t--- sat/cent have the same ID\n", 
               galaxies[ngal].SnapNum, galaxies[ngal].Type, sat_halo, mother_halo);
        return -1.0;
    }
    
    const double coulomb = log(1.0 + halos[mother_halo].Len / ((double) halos[sat_halo].Len) );

    const double SatelliteMass = get_virial_mass(sat_halo, halos) + galaxies[ngal].StellarMass + galaxies[ngal].ColdGas;
    const double SatelliteRadius = get_virial_radius(mother_halo, halos);

    if(SatelliteMass > 0.0 && coulomb > 0.0 && halos[sat_halo].Len >= MinNumPartSatHalo) {
        mergtime = 2.0 *
            1.17 * SatelliteRadius * SatelliteRadius * get_virial_velocity(mother_halo, halos) / (coulomb * run_params.G * SatelliteMass);
    } else {
        mergtime = -1.0;
    }

    return mergtime;

}



void deal_with_galaxy_merger(const int p, const int merger_centralgal, const int centralgal,
                             const double time, const double dt, const int halonr, const int step,
                             struct GALAXY *galaxies)
{
    double mi, ma, mass_ratio;

    // calculate mass ratio of merging galaxies 
    if(galaxies[p].StellarMass + galaxies[p].ColdGas <
       galaxies[merger_centralgal].StellarMass + galaxies[merger_centralgal].ColdGas) {
        mi = galaxies[p].StellarMass + galaxies[p].ColdGas;
        ma = galaxies[merger_centralgal].StellarMass + galaxies[merger_centralgal].ColdGas;
    } else {
        mi = galaxies[merger_centralgal].StellarMass + galaxies[merger_centralgal].ColdGas;
        ma = galaxies[p].StellarMass + galaxies[p].ColdGas;
    }

    if(ma > 0) {
        mass_ratio = mi / ma;
    } else {
        mass_ratio = 1.0;
    }

    add_galaxies_together(merger_centralgal, p, galaxies);

    // grow black hole through accretion from cold disk during mergers, a la Kauffmann & Haehnelt (2000) 
    if(run_params.AGNrecipeOn) {
        grow_black_hole(merger_centralgal, mass_ratio, galaxies);
    }
  
    // starburst recipe similar to Somerville et al. 2001
    collisional_starburst_recipe(mass_ratio, merger_centralgal, centralgal, time, dt, halonr, 0, step, galaxies);

    if(mass_ratio > 0.1) {
		galaxies[merger_centralgal].TimeOfLastMinorMerger = time;
    }

    if(mass_ratio > run_params.ThreshMajorMerger) {
        make_bulge_from_burst(merger_centralgal, galaxies);
        galaxies[merger_centralgal].TimeOfLastMajorMerger = time;
        galaxies[p].mergeType = 2;  // mark as major merger
    } else {
        galaxies[p].mergeType = 1;  // mark as minor merger
    }

}



void grow_black_hole(const int merger_centralgal, const double mass_ratio, struct GALAXY *galaxies)
{
    double BHaccrete, metallicity;

    if(galaxies[merger_centralgal].ColdGas > 0.0) {
        BHaccrete = run_params.BlackHoleGrowthRate * mass_ratio / 
            (1.0 + pow(280.0 / galaxies[merger_centralgal].Vvir, 2.0)) * galaxies[merger_centralgal].ColdGas;
        
        // cannot accrete more gas than is available! 
        if(BHaccrete > galaxies[merger_centralgal].ColdGas) {
            BHaccrete = galaxies[merger_centralgal].ColdGas;
        }
        
        metallicity = get_metallicity(galaxies[merger_centralgal].ColdGas, galaxies[merger_centralgal].MetalsColdGas);
        galaxies[merger_centralgal].BlackHoleMass += BHaccrete;
        galaxies[merger_centralgal].ColdGas -= BHaccrete;
        galaxies[merger_centralgal].MetalsColdGas -= metallicity * BHaccrete;
        
        galaxies[merger_centralgal].QuasarModeBHaccretionMass += BHaccrete;
        
        quasar_mode_wind(merger_centralgal, BHaccrete, galaxies);
    }
}



void quasar_mode_wind(const int gal, const double BHaccrete, struct GALAXY *galaxies)
{
    // work out total energies in quasar wind (eta*m*c^2), cold and hot gas (1/2*m*Vvir^2)
    const double quasar_energy = run_params.QuasarModeEfficiency * 0.1 * BHaccrete * (C / run_params.UnitVelocity_in_cm_per_s) * (C / run_params.UnitVelocity_in_cm_per_s);
    const double cold_gas_energy = 0.5 * galaxies[gal].ColdGas * galaxies[gal].Vvir * galaxies[gal].Vvir;
    const double hot_gas_energy = 0.5 * galaxies[gal].HotGas * galaxies[gal].Vvir * galaxies[gal].Vvir;
   
    // compare quasar wind and cold gas energies and eject cold
    if(quasar_energy > cold_gas_energy) {
        galaxies[gal].EjectedMass += galaxies[gal].ColdGas;
        galaxies[gal].MetalsEjectedMass += galaxies[gal].MetalsColdGas;
        
        galaxies[gal].ColdGas = 0.0;
        galaxies[gal].MetalsColdGas = 0.0;
    }
  
    // compare quasar wind and cold+hot gas energies and eject hot
    if(quasar_energy > cold_gas_energy + hot_gas_energy) {
        galaxies[gal].EjectedMass += galaxies[gal].HotGas;
        galaxies[gal].MetalsEjectedMass += galaxies[gal].MetalsHotGas;
        
        galaxies[gal].HotGas = 0.0;
        galaxies[gal].MetalsHotGas = 0.0;
    }
}



void add_galaxies_together(const int t, const int p, struct GALAXY *galaxies)
{
    galaxies[t].ColdGas += galaxies[p].ColdGas;
    galaxies[t].MetalsColdGas += galaxies[p].MetalsColdGas;
  
    galaxies[t].StellarMass += galaxies[p].StellarMass;
    galaxies[t].MetalsStellarMass += galaxies[p].MetalsStellarMass;

    galaxies[t].HotGas += galaxies[p].HotGas;
    galaxies[t].MetalsHotGas += galaxies[p].MetalsHotGas;
  
    galaxies[t].EjectedMass += galaxies[p].EjectedMass;
    galaxies[t].MetalsEjectedMass += galaxies[p].MetalsEjectedMass;
  
    galaxies[t].ICS += galaxies[p].ICS;
    galaxies[t].MetalsICS += galaxies[p].MetalsICS;

    galaxies[t].BlackHoleMass += galaxies[p].BlackHoleMass;

    // add merger to bulge
    galaxies[t].BulgeMass += galaxies[p].StellarMass;
    galaxies[t].MetalsBulgeMass += galaxies[p].MetalsStellarMass;		

    for(int step = 0; step < STEPS; step++) {
        galaxies[t].SfrBulge[step] += galaxies[p].SfrDisk[step] + galaxies[p].SfrBulge[step];
        galaxies[t].SfrBulgeColdGas[step] += galaxies[p].SfrDiskColdGas[step] + galaxies[p].SfrBulgeColdGas[step];
        galaxies[t].SfrBulgeColdGasMetals[step] += galaxies[p].SfrDiskColdGasMetals[step] + galaxies[p].SfrBulgeColdGasMetals[step];
    }
}



void make_bulge_from_burst(const int p, struct GALAXY *galaxies)
{
    // generate bulge 
    galaxies[p].BulgeMass = galaxies[p].StellarMass;
    galaxies[p].MetalsBulgeMass = galaxies[p].MetalsStellarMass;

    // update the star formation rate 
    for(int step = 0; step < STEPS; step++) {
        galaxies[p].SfrBulge[step] += galaxies[p].SfrDisk[step];
        galaxies[p].SfrBulgeColdGas[step] += galaxies[p].SfrDiskColdGas[step];
        galaxies[p].SfrBulgeColdGasMetals[step] += galaxies[p].SfrDiskColdGasMetals[step];
        galaxies[p].SfrDisk[step] = 0.0;
        galaxies[p].SfrDiskColdGas[step] = 0.0;
        galaxies[p].SfrDiskColdGasMetals[step] = 0.0;
    }
}



void collisional_starburst_recipe(const double mass_ratio, const int merger_centralgal, const int centralgal,
                                  const double time, const double dt, const int halonr, const int mode, const int step,
                                  struct GALAXY *galaxies)
{
    double stars, reheated_mass, ejected_mass, fac, metallicity, eburst;

    // This is the major and minor merger starburst recipe of Somerville et al. 2001. 
    // The coefficients in eburst are taken from TJ Cox's PhD thesis and should be more accurate then previous. 

    // the bursting fraction 
    if(mode == 1) {
        eburst = mass_ratio;
    } else {
        eburst = 0.56 * pow(mass_ratio, 0.7);
    }

    stars = eburst * galaxies[merger_centralgal].ColdGas;
    if(stars < 0.0) {
        stars = 0.0;
    }

    // this bursting results in SN feedback on the cold/hot gas 
    if(run_params.SupernovaRecipeOn == 1) {
        reheated_mass = run_params.FeedbackReheatingEpsilon * stars;
    } else {
        reheated_mass = 0.0;
    }

	assert(reheated_mass >= 0.0);

    // can't use more cold gas than is available! so balance SF and feedback 
    if((stars + reheated_mass) > galaxies[merger_centralgal].ColdGas) {
        fac = galaxies[merger_centralgal].ColdGas / (stars + reheated_mass);
        stars *= fac;
        reheated_mass *= fac;
    }

    // determine ejection
    if(run_params.SupernovaRecipeOn == 1) {
        if(galaxies[centralgal].Vvir > 0.0) {
            ejected_mass = 
                (run_params.FeedbackEjectionEfficiency * (run_params.EtaSNcode * run_params.EnergySNcode) / (galaxies[centralgal].Vvir * galaxies[centralgal].Vvir) - 
                 run_params.FeedbackReheatingEpsilon) * stars;
        } else {
            ejected_mass = 0.0;
        }
		
        if(ejected_mass < 0.0) {
            ejected_mass = 0.0;
        }
    } else {
        ejected_mass = 0.0;
    }

    // starbursts add to the bulge
    galaxies[merger_centralgal].SfrBulge[step] += stars / dt;
    galaxies[merger_centralgal].SfrBulgeColdGas[step] += galaxies[merger_centralgal].ColdGas;
    galaxies[merger_centralgal].SfrBulgeColdGasMetals[step] += galaxies[merger_centralgal].MetalsColdGas;

    metallicity = get_metallicity(galaxies[merger_centralgal].ColdGas, galaxies[merger_centralgal].MetalsColdGas);
    update_from_star_formation(merger_centralgal, stars, metallicity, galaxies);

    galaxies[merger_centralgal].BulgeMass += (1 - run_params.RecycleFraction) * stars;
    galaxies[merger_centralgal].MetalsBulgeMass += metallicity * (1 - run_params.RecycleFraction) * stars;

    // recompute the metallicity of the cold phase
    metallicity = get_metallicity(galaxies[merger_centralgal].ColdGas, galaxies[merger_centralgal].MetalsColdGas);

    // update from feedback 
    update_from_feedback(merger_centralgal, centralgal, reheated_mass, ejected_mass, metallicity, galaxies);

    // check for disk instability
    if(run_params.DiskInstabilityOn && mode == 0) {
        if(mass_ratio < run_params.ThreshMajorMerger) {
            check_disk_instability(merger_centralgal, centralgal, halonr, time, dt, step, galaxies);
        }
    }

    // formation of new metals - instantaneous recycling approximation - only SNII 
    if(galaxies[merger_centralgal].ColdGas > 1e-8 && mass_ratio < run_params.ThreshMajorMerger) {
        const double FracZleaveDiskVal = run_params.FracZleaveDisk * exp(-1.0 * galaxies[centralgal].Mvir / 30.0);  // Krumholz & Dekel 2011 Eq. 22
        galaxies[merger_centralgal].MetalsColdGas += run_params.Yield * (1.0 - FracZleaveDiskVal) * stars;
        galaxies[centralgal].MetalsHotGas += run_params.Yield * FracZleaveDiskVal * stars;
        // galaxies[centralgal].MetalsEjectedMass += run_params.Yield * FracZleaveDiskVal * stars;
    } else {
        galaxies[centralgal].MetalsHotGas += run_params.Yield * stars;
        // galaxies[centralgal].MetalsEjectedMass += run_params.Yield * stars;
    }
}



void disrupt_satellite_to_ICS(const int centralgal, const int gal, struct GALAXY *galaxies)
{  
    galaxies[centralgal].HotGas += galaxies[gal].ColdGas + galaxies[gal].HotGas;
    galaxies[centralgal].MetalsHotGas += galaxies[gal].MetalsColdGas + galaxies[gal].MetalsHotGas;
  
    galaxies[centralgal].EjectedMass += galaxies[gal].EjectedMass;
    galaxies[centralgal].MetalsEjectedMass += galaxies[gal].MetalsEjectedMass;
  
    galaxies[centralgal].ICS += galaxies[gal].ICS;
    galaxies[centralgal].MetalsICS += galaxies[gal].MetalsICS;

    galaxies[centralgal].ICS += galaxies[gal].StellarMass;
    galaxies[centralgal].MetalsICS += galaxies[gal].MetalsStellarMass;
  
    // what should we do with the disrupted satellite BH?
    galaxies[gal].mergeType = 4;  // mark as disruption to the ICS
}




