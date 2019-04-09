#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"

#include "model_starformation_and_feedback.h"
#include "model_misc.h"
#include "model_dust.h"
#include "model_disk_instability.h"

void starformation_and_feedback(const int p, const int centralgal, const double time, const double dt, const int halonr, const int step, struct GALAXY *galaxies)
{
    double reff, tdyn, strdot, stars, ejected_mass, fac, metallicity, DTG;
    double cold_crit;
    double area, sigma, sigma_crit, sfr_ff;
  
    // Initialise variables
    strdot = 0.0;

    // update H2 and HI gas
    update_H2_HI(p, galaxies);

    // star formation recipes 
     if(run_params.SFprescription == 0)
	 {
	        // we take the typical star forming region as 3.0*r_s using the Milky Way as a guide
	        reff = 3.0 * galaxies[p].DiskScaleRadius;
        	tdyn = reff / galaxies[p].Vvir;
        
       		// from Kauffmann (1996) eq7 x piR^2, (Vvir in km/s, reff in Mpc/h) in units of 10^10Msun/h 
        	cold_crit = 0.19 * galaxies[p].Vvir * reff;
        	if(galaxies[p].ColdGas > cold_crit && tdyn > 0.0) 
		{
        	  strdot = run_params.SfrEfficiency * (galaxies[p].ColdGas - cold_crit) / tdyn;
        	} 
     		else 
		{
            	  strdot = 0.0;
        	}
	}
    else if(run_params.SFprescription == 1) 
    {
        // make stars only from H2
	area = M_PI * 9.0 * galaxies[p].DiskScaleRadius * galaxies[p].DiskScaleRadius;
        strdot = run_params.UnitTime_in_s / SEC_PER_MEGAYEAR * run_params.SfrEfficiency * galaxies[p].f_H2 * galaxies[p].ColdGas;
    } 
    else if(run_params.SFprescription == 2) 
    {
	//Krumholz & McKee 2009
	area = M_PI * 9.0 * galaxies[p].DiskScaleRadius * galaxies[p].DiskScaleRadius;  
	sigma = galaxies[p].ColdGas / area;
	sigma_crit = sigma * 0.73 / 8500;
	//galaxies[p].sigma_crit = sigma_crit;
	double tff = 2.6 * 1000;
	if(sigma_crit < 1 && sigma_crit > 0)
	 {
		sfr_ff = pow(sigma_crit, -0.33);
		//galaxies[p].sigma_crit = (sfr_ff/tff);
		//strdot = run_params.UnitTime_in_s / SEC_PER_MEGAYEAR * 0.02  * galaxies[p].f_H2 * galaxies[p].ColdGas;
		strdot = galaxies[p].f_H2 * galaxies[p].ColdGas * (sfr_ff / tff) * run_params.UnitTime_in_s / SEC_PER_MEGAYEAR ;
	 }	
	else if(sigma_crit > 1) 
	{
		sfr_ff = pow(sigma_crit, 0.33);
		//galaxies[p].sigma_crit = (sfr_ff/tff);
		//strdot = run_params.UnitTime_in_s / SEC_PER_MEGAYEAR * 0.02 * galaxies[p].f_H2 * galaxies[p].ColdGas;
		strdot = galaxies[p].f_H2 * galaxies[p].ColdGas * (sfr_ff / tff) * run_params.UnitTime_in_s / SEC_PER_MEGAYEAR;
	}
    }
    else 
    {
        printf("No star formation prescription selected!\n");
        ABORT(0);
    }
   
    stars = strdot * dt;
    if(stars < 0.0) {
        stars = 0.0;
    }

    double reheated_mass = (run_params.SupernovaRecipeOn == 1) ? run_params.FeedbackReheatingEpsilon * stars: 0.0;

	assert(reheated_mass >= 0.0);

    // cant use more cold gas than is available! so balance SF and feedback 
    if((stars + reheated_mass) > galaxies[p].ColdGas && (stars + reheated_mass) > 0.0) {
        fac = galaxies[p].ColdGas / (stars + reheated_mass);
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

    // update the star formation rate 
    galaxies[p].SfrDisk[step] += stars / dt;
    galaxies[p].SfrDiskColdGas[step] = galaxies[p].ColdGas;
    galaxies[p].SfrDiskColdGasMetals[step] = galaxies[p].MetalsColdGas;

    // update new variables
    galaxies[p].Sfr[galaxies[p].SnapNum] += stars / dt / STEPS;
    
    // update for star formation 
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);
    DTG = get_DTG(galaxies[p].ColdGas, galaxies[p].ColdDust);
    update_from_star_formation(p, stars, metallicity, DTG, galaxies);

    // recompute the metallicity of the cold phase
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);
    DTG = get_DTG(galaxies[p].ColdGas, galaxies[p].ColdDust);

    // update from SN feedback 
    update_from_feedback(p, centralgal, reheated_mass, ejected_mass, metallicity, DTG, galaxies);

    // check for disk instability
    if(run_params.DiskInstabilityOn) {
        check_disk_instability(p, centralgal, halonr, time, dt, step, galaxies);
    }

    //formation of new metals and dust
    if(run_params.MetalYieldsOn == 0)
    { // instantaneous recycling approximation - only SNII 
    if(galaxies[p].ColdGas > 1.0e-8) {
        const double FracZleaveDiskVal = run_params.FracZleaveDisk * exp(-1.0 * galaxies[centralgal].Mvir / 30.0);  // Krumholz & Dekel 2011 Eq. 22
        galaxies[p].MetalsColdGas += run_params.Yield * (1.0 - FracZleaveDiskVal) * stars;
        galaxies[centralgal].MetalsHotGas += run_params.Yield * FracZleaveDiskVal * stars;
        // galaxies[centralgal].MetalsEjectedMass += run_params.Yield * FracZleaveDiskVal * stars;

    } else {
        galaxies[centralgal].MetalsHotGas += run_params.Yield * stars;
        // galaxies[centralgal].MetalsEjectedMass += run_params.Yield * stars;
      }
    }
    else if(run_params.MetalYieldsOn == 1)
    { // self consistent yields - AGB, SNII, and SNIa
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);
    produce_metals_dust(metallicity, dt, p, centralgal, step, galaxies);
    }
    else
    {
        printf("No metals formation prescription selected!\n");
        ABORT(0);
    }

    //update for dust accretion
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);
    accrete_dust(metallicity, dt, p, step, galaxies);
   
    //update for dust destruction
    metallicity = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas);
    destruct_dust(metallicity, stars, dt, p, step, galaxies);
   

}



void update_from_star_formation(const int p, const double stars, const double metallicity, const double DTG, struct GALAXY *galaxies)
{
    const double RecycleFraction = run_params.RecycleFraction;
    // update gas and metals from star formation 
    galaxies[p].ColdGas -= (1 - RecycleFraction) * stars;
    galaxies[p].MetalsColdGas -= metallicity * (1 - RecycleFraction) * stars;
    galaxies[p].ColdDust -= DTG * (1 - RecycleFraction) * stars;
    galaxies[p].StellarMass += (1 - RecycleFraction) * stars;
    galaxies[p].MetalsStellarMass += metallicity * (1 - RecycleFraction) * stars;
    galaxies[p].MetalsStellarMass += DTG * (1 - RecycleFraction) * stars;
}



void update_from_feedback(const int p, const int centralgal, const double reheated_mass, double ejected_mass, const double metallicity, const double DTG,  struct GALAXY *galaxies)
{
	assert(!(reheated_mass > galaxies[p].ColdGas && reheated_mass > 0.0));

    if(run_params.SupernovaRecipeOn == 1) {
        galaxies[p].ColdGas -= reheated_mass;
        galaxies[p].MetalsColdGas -= metallicity * reheated_mass;
	galaxies[p].ColdDust -= DTG * reheated_mass;
 
        galaxies[centralgal].HotGas += reheated_mass;
        galaxies[centralgal].MetalsHotGas += metallicity * reheated_mass;
        galaxies[centralgal].HotDust += DTG * reheated_mass;

        if(ejected_mass > galaxies[centralgal].HotGas) {
            ejected_mass = galaxies[centralgal].HotGas;
        }
        const double metallicityHot = get_metallicity(galaxies[centralgal].HotGas, galaxies[centralgal].MetalsHotGas);
	const double DTGHot = get_DTG(galaxies[centralgal].HotGas, galaxies[centralgal].HotDust);

        galaxies[centralgal].HotGas -= ejected_mass;
        galaxies[centralgal].MetalsHotGas -= metallicityHot * ejected_mass;
	galaxies[centralgal].HotDust -= DTGHot * ejected_mass;

        galaxies[centralgal].EjectedMass += ejected_mass;
        galaxies[centralgal].MetalsEjectedMass += metallicityHot * ejected_mass;
	galaxies[centralgal].EjectedDust += DTGHot * ejected_mass;
        
        galaxies[p].OutflowRate += reheated_mass;    
    }
}

void update_H2_HI(const int p, struct GALAXY *galaxies)
{
        double area, f_H2_HI ;
        
        if(galaxies[p].Vvir>0.0)
	{
		area = M_PI * 9 * galaxies[p].DiskScaleRadius * galaxies[p].DiskScaleRadius;
 
		if(run_params.H2prescription == 0)
		{

			double s, Zp, chi, cf, Sigma_comp0, Tau_c;
			Zp = get_metallicity(galaxies[p].ColdGas, galaxies[p].MetalsColdGas) / 0.02;
			galaxies[p].Zp = Zp;
			double factor = run_params.H2ClumpFactor;
			double exp = run_params.H2ClumpExp;

			if (Zp>0.01 && Zp<1) // Fu et al.2013 
			{
				cf = factor*pow(Zp, -exp);
				galaxies[p].cf = factor*pow(Zp, -exp);
			}
			else if(Zp>=1) 
			{
				cf = factor;
				galaxies[p].cf = factor;
			}
			else 
			{
				cf = factor*pow(0.01, -exp);
				galaxies[p].cf = factor*pow(0.01, -exp);
			}
			galaxies[p].cf = cf;
			Sigma_comp0 = cf*galaxies[p].ColdGas/area;
			Tau_c = 320 * Zp * Sigma_comp0 * run_params.UnitMass_in_g / run_params.UnitLength_in_cm / run_params.UnitLength_in_cm * run_params.Hubble_h;
			chi = 3.1 * (1 + 3.1 * pow(Zp, 0.365)) / 4.1;
			s = log(1 + 0.6*chi + 0.01*chi*chi) / (0.6*Tau_c);

			if(s<5)
			{
				galaxies[p].f_H2 = 1.0 - 0.75*s/(1 + 0.25*s); //This is H2/(H2+HI)
				f_H2_HI = 1.0 / (1.0/galaxies[p].f_H2 - 1.0);
			}
			else
				f_H2_HI = 0.0;
		}

		else
		{
			double f_sigma, Pressure; 
			double P0 = 5.93e-12 / run_params.UnitMass_in_g * run_params.UnitLength_in_cm * run_params.UnitTime_in_s * run_params.UnitTime_in_s;
			
			//double rstar = 1.68 * galaxies[p].DiskScaleRadius; //Cole08
			//double hstar = rstar / 7.3; //Lagos18
			//f_sigma = 1.0e6/run_params.UnitVelocity_in_cm_per_s / sqrt(M_PI*run_params.G*hstar*galaxies[p].StellarMass/area); //Lagos18
			//f_sigma = 1.1e6/run_params.UnitVelocity_in_cm_per_s / (0.5*galaxies[p].Vvir*exp(-3/2)); 
			// ratio of gas velocity dispersion to stars, assuming gas is always 11 km/s and stellar disk radius is 3 times disk scale radius (Stevens17)
			//Pressure = 0.5*M_PI*run_params.G * (galaxies[p].ColdGas / area) * (galaxies[p].ColdGas / area);
			
			f_sigma = 0.4; // Elmegreen89
			Pressure = 0.5*M_PI*run_params.G * galaxies[p].ColdGas * (galaxies[p].ColdGas + f_sigma*galaxies[p].StellarMass) / (area*area);
			galaxies[p].Pressure = Pressure;
			f_H2_HI = pow(Pressure/P0, run_params.H2Exp);	
		}
		

		if(f_H2_HI > 0.0)
		{
			assert(galaxies[p].MetalsColdGas <= galaxies[p].ColdGas);
			galaxies[p].f_H2 = 0.75 * 1.0/(1.0/f_H2_HI + 1) * (1 - galaxies[p].MetalsColdGas/galaxies[p].ColdGas) / 1.3; //This is H2/ColdGas
			galaxies[p].f_HI = galaxies[p].f_H2/f_H2_HI;
		}
		else
		{
			galaxies[p].f_H2 = 0.0;
			galaxies[p].f_HI = 0.75 * (1 - galaxies[p].MetalsColdGas/galaxies[p].ColdGas) / 1.3;
		}
		
	}	
}
