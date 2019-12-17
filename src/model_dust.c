#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#include "core_allvars.h"
#include "model_dust.h"
#include "model_misc.h"

double integrate_arr(const double arr1[MAX_STRING_LEN], const double arr2[MAX_STRING_LEN], const int npts, const double lower_limit, const double upper_limit);
double interpolate_arr(const double arr1[MAX_STRING_LEN], const double arr2[MAX_STRING_LEN], const int npts, const int xi);
double compute_imf (const double m);
double compute_taum (const double m);

void produce_metals_dust(const double metallicity, const double dt, const int p, const int centralgal, const int step, struct GALAXY *galaxies)
{
  double A = run_params.BinaryFraction;

  double yield;
  double yCr_snia, yFe_snia, yNi_snia;
  double yC_agb, yN_agb, yO_agb;
  double yC_sn, yO_sn, yMg_sn, ySi_sn, yS_sn, yCa_sn, yFe_sn;
  double Cr_snia, Fe_snia, Ni_snia;
  double C_agb, N_agb, O_agb;
  double C_sn, O_sn, Mg_sn, Si_sn, S_sn, Ca_sn, Fe_sn;
  double yCagb[run_params.countagb], yNagb[run_params.countagb], yOagb[run_params.countagb];
  double m_agb[run_params.countagb];
  double yCsn[run_params.countsn], yOsn[run_params.countsn], yMgsn[run_params.countsn], ySisn[run_params.countsn], ySsn[run_params.countsn], yCasn[run_params.countsn], yFesn[run_params.countsn];
  double m_sn[run_params.countsn];

  int i, j;
  double phi, taum, time, sfr;
  double gamma = 2.0; //to compute binary mass function for SN Ia  
  double low_agb = 1; //lower limit for stars that ends up as AGB, Msun
  double low_binary = 3; //lower limit for binary star, Msun
  double up_agb = 6; //upper limit for stars that ends up as AGB, Msun
  double low_sn = run_params.msn[0]; //lower limit for stars that ends up as SN II, Msun
  double up_binary = 16; //upper limit for binary stars, Msun
  double up_sn = 40; //upper limit for stars that ends up as SN II, Msun
  double age[SNAPLEN], sfh[SNAPLEN];

  for (i=0; i<SNAPLEN; i++)
  {
        age[i] = run_params.lbtime[i]; //change the variable name to represent the age of universe instead of lbtime
        sfh[i] = galaxies[p].Sfr[i];
  }


/* the following computation is from eq. 9 Arrigoni et al. 2010 */

  // SN Ia //
  int count = 20; //any nymber will do
  double mu[count]; // M2/Mbinary
  double mbin[count]; //total mass of binary
  double fmu, yCrmu[count], yFemu[count], yNimu[count]; //to integrate over mu
  double yCrphi[count], yFephi[count], yNiphi[count];
  double max_mu = 0.5; //maximum value of mu

  for (i=0; i<count; i++){
        mbin[i] = low_binary + ((up_binary - low_binary) / (count-i));
        mu[i] = max_mu/(count-i);
        fmu = pow(2, 1+gamma) * (1+gamma) * pow(mu[i], gamma); //eq. 10 Arrigoni et al. 2010
        taum = compute_taum(mu[i] * mbin[i]);
        time = age[galaxies[p].SnapNum] - taum;
        if(time < run_params.lbtime[0]){
                sfr=0;
        }
        else {
                sfr = interpolate_arr(age, sfh, SNAPLEN, time);
        }
        yCrmu[i] = fmu * sfr * run_params.qCrsnia;
        yFemu[i] = fmu * sfr * run_params.qFesnia;
        yNimu[i] = fmu * sfr * run_params.qNisnia;
  }

  double yCr = integrate_arr(mu, yCrmu, count, max_mu/count, max_mu);
  double yFe = integrate_arr(mu, yFemu, count, max_mu/count, max_mu);
  double yNi = integrate_arr(mu, yNimu, count, max_mu/count, max_mu);
  for (i=0; i<count; i++){
        yCrphi[i] = yCr * compute_imf(mbin[i]);
        yFephi[i] = yFe * compute_imf(mbin[i]);
        yNiphi[i] = yNi * compute_imf(mbin[i]);
  //printf("SNIa done \n");
  }


  // AGB //
    if(run_params.AGBYields == 0)
    {	
	i = j = 0;
	double Z_std[METALGRID] = {0.0, 1e-4, 4e-4, 4e-3, 8e-3, 0.02, 0.05};
	double sfrz = metallicity;
	if (sfrz < Z_std[0]){
		sfrz = Z_std[0];}
	else if (sfrz > Z_std[6]){
		sfrz = Z_std[6];}

	for (i=0; i<METALGRID; i++) {
		if(Z_std[i] <= sfrz && Z_std[i] > sfrz) {
			j = i;
		}
	}

	//double yCagb[run_params.countagb], yNagb[run_params.countagb], yOagb[run_params.countagb];
	//double m_agb[run_params.countagb];
	for(i=0; i<run_params.countagb; i++){
		if(run_params.magb[i] != 0) {
			m_agb[i] = run_params.magb[i];
			phi = compute_imf(run_params.magb[i]);
			taum = compute_taum(run_params.magb[i]);
			time = age[galaxies[p].SnapNum] - taum;
			if(time < run_params.lbtime[0]){
				sfr=0;
			}
			else {
				 sfr = interpolate_arr(age, sfh, SNAPLEN, time);
			}
			yCagb[i] = run_params.qCagb[i][j] * phi * sfr;
        		yNagb[i] = run_params.qNagb[i][j] * phi * sfr;
        		yOagb[i] = run_params.qOagb[i][j] * phi * sfr;	
		}
		else {
        		yCagb[i] = 0;
        		yNagb[i] = 0;
        		yOagb[i] = 0;			
		}
	}
    //printf("AGB done \n");
    } 

  // SN II // 
    if(run_params.SNIIYields == 0) //Woosley & Weaver 1995
    {
	i = j = 0;
	double Z_std[METALGRID] = {0.0, 1e-4, 4e-4, 4e-3, 8e-3, 0.02, 0.05};
	double sfrz = metallicity;
        if (sfrz < Z_std[0]){
                sfrz = Z_std[0];}
        else if (sfrz > Z_std[6]){
                sfrz = Z_std[6];}

        for (i=0; i<METALGRID; i++) {
                if(Z_std[i] <= sfrz && Z_std[i] > sfrz) {
                        j = i;
                }
        }

      	for(i=0; i<run_params.countsn; i++){
        	if(run_params.msn[i] != 0) {
         		m_sn[i] = run_params.msn[i];
         		phi = compute_imf(run_params.msn[i]);
          		taum = compute_taum(run_params.msn[i]);
          		time = age[galaxies[p].SnapNum] - taum;
          		if(time < run_params.lbtime[0]){
                		sfr=0;
          		}
         		 else {
                	sfr = interpolate_arr(age, sfh, SNAPLEN, time);
	  		}
          		yCsn[i] = run_params.qCsn[i][j] * phi * sfr;
          		yOsn[i] = run_params.qOsn[i][j] * phi * sfr;
          		yMgsn[i] = run_params.qMgsn[i][j] * phi * sfr;
          		ySisn[i] = run_params.qSisn[i][j] * phi * sfr;
          		ySsn[i] = run_params.qSsn[i][j] * phi * sfr;
          		yCasn[i] = run_params.qCasn[i][j] * phi * sfr;
          		yFesn[i] = run_params.qFesn[i][j] * phi * sfr;
        	}
        	else {
        	yCsn[i] = 0;
        	yOsn[i] = 0;
        	yMgsn[i] = 0;
        	ySisn[i] = 0;
        	ySsn[i] = 0;
        	yCasn[i] = 0;
        	yFesn[i] = 0;
        	}
    	}
     //printf("SNII done \n");
     }

    else if(run_params.SNIIYields == 1) //Nomoto et al. 2006
    {
        i = j = 0;
        double Z_std[METALGRID] = {0.0, 0.001, 0004, 0.02};
        double sfrz = metallicity;
        if (sfrz < Z_std[0]){
                sfrz = Z_std[0];}
        else if (sfrz > Z_std[3]){
                sfrz = Z_std[3];}
	
        for (i=0; i<METALGRID; i++) {
                if(Z_std[i] <= sfrz && Z_std[i] > sfrz) {
                        j = i;
                }
        }

        for(i=0; i<run_params.countsn; i++){
                if(run_params.msn[i] != 0) {
                        m_sn[i] = run_params.msn[i];
                        phi = compute_imf(run_params.msn[i]);
                        taum = compute_taum(run_params.msn[i]);
                        time = age[galaxies[p].SnapNum] - taum;
                        if(time < run_params.lbtime[0]){
                                sfr=0;
                        }
                         else {
                        sfr = interpolate_arr(age, sfh, SNAPLEN, time);
                        }
                        yCsn[i] = run_params.qCsn[i][j] * phi * sfr;
                        yOsn[i] = run_params.qOsn[i][j] * phi * sfr;
                        yMgsn[i] = run_params.qMgsn[i][j] * phi * sfr;
                        ySisn[i] = run_params.qSisn[i][j] * phi * sfr;
                        ySsn[i] = run_params.qSsn[i][j] * phi * sfr;
                        yCasn[i] = run_params.qCasn[i][j] * phi * sfr;
                        yFesn[i] = run_params.qFesn[i][j] * phi * sfr;
                }
                else {
                yCsn[i] = 0;
                yOsn[i] = 0;
                yMgsn[i] = 0;
                ySisn[i] = 0;
                ySsn[i] = 0;
                yCasn[i] = 0;
                yFesn[i] = 0;
                }
        }
     //printf("SNII done \n");
     }

    //printf("start integration");
    yCr_snia = A * integrate_arr(mbin, yCrphi, count, mbin[0], up_binary);
    yNi_snia = A * integrate_arr(mbin, yNiphi, count, mbin[0], up_binary);
    yFe_snia = A * integrate_arr(mbin, yFephi, count, mbin[0], up_binary);
    yC_agb = (1-A) * integrate_arr(m_agb, yCagb, run_params.countagb, low_binary, up_agb) + integrate_arr(m_agb, yCagb, run_params.countagb, low_agb, low_binary);
    yN_agb = (1-A) * integrate_arr(m_agb, yNagb, run_params.countagb, low_binary, up_agb) + integrate_arr(m_agb, yNagb, run_params.countagb, low_agb, low_binary);
    yO_agb = (1-A) * integrate_arr(m_agb, yOagb, run_params.countagb, low_binary, up_agb) + integrate_arr(m_agb, yOagb, run_params.countagb, low_agb, low_binary);
    yC_sn =  (1-A) * integrate_arr(m_sn, yCsn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, yCsn, run_params.countsn, up_binary, up_sn);
    yO_sn =  (1-A) * integrate_arr(m_sn, yOsn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, yOsn, run_params.countsn, up_binary, up_sn);
    yMg_sn =  (1-A) * integrate_arr(m_sn, yMgsn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, yMgsn, run_params.countsn, up_binary, up_sn);
    ySi_sn =  (1-A) * integrate_arr(m_sn, ySisn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, ySisn, run_params.countsn, up_binary, up_sn);
    yS_sn =  (1-A) * integrate_arr(m_sn, ySsn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, ySsn, run_params.countsn, up_binary, up_sn);
    yCa_sn =  (1-A) * integrate_arr(m_sn, yCasn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, yCasn, run_params.countsn, up_binary, up_sn);
    yFe_sn =  (1-A) * integrate_arr(m_sn, yFesn, run_params.countsn, low_sn, up_binary) + integrate_arr(m_sn, yFesn, run_params.countsn, up_binary, up_sn);
    //printf("end integration");

    double yield_agb = yC_agb + yN_agb + yO_agb;
    double yield_snia = yCr_snia + yNi_snia + yFe_snia;
    double yield_sn = yC_sn + yO_sn + yMg_sn + ySi_sn + yS_sn + yCa_sn + yFe_sn;
    yield = (yield_agb + yield_snia + yield_sn);

//overall metallicity
  if(galaxies[p].ColdGas > 5.0e-2) {
        galaxies[p].MetalsColdGas += yield * dt;
  }
  else {
        galaxies[centralgal].MetalsHotGas += yield * dt;
  }
  XPRINT(galaxies[p].MetalsColdGas <= galaxies[p].ColdGas, "metallicity = %.3f, cold gas mass = %.3e, galaxy ID = %i \n", galaxies[p].MetalsColdGas / galaxies[p].ColdGas, galaxies[p].ColdGas, galaxies[p].GalaxyNr);


//mass of each element formed in each production channel, should satisfy same condition with overall metallicity
  if(galaxies[p].ColdGas > 1.0e-2) {
        Cr_snia = yCr_snia * dt;
        Fe_snia = yFe_snia * dt;
        Ni_snia = yNi_snia * dt;
        C_agb = yC_agb * dt;
        N_agb = yN_agb * dt;
        O_agb = yO_agb * dt;
        C_sn = yC_sn * dt;
        O_sn = yO_sn * dt;
        Mg_sn = yMg_sn * dt;
        Si_sn = ySi_sn * dt;
        S_sn = yS_sn * dt;
        Ca_sn = yCa_sn * dt;
        Fe_sn = yFe_sn * dt;
	

        double dustdot = 0;

        //produce dust
        //double dust_agb, dust_sn, dust_snia;
        double delta_agb = 0.2;  //should be free parameter!! (value based on Popping et al. 2017)
        double delta_sn = 0.2;  //should be free parameter!! (value based on Popping et al. 2017)
        double delta_snia = 0.15;  //should be free parameter!! (value based on Popping et al. 2017)

        assert(dt > 0 && "dt must be greater than 0");

        /* eq 4 and eq 5 Popping et al. 2017 */
        //from AGB
        if (C_agb/O_agb > 1) {
           dustdot += delta_agb * (C_agb - 0.75*O_agb) / dt;
        }
        else {
           dustdot += delta_agb * (C_agb + N_agb + O_agb) / dt;
        }

        /* eq 6 Popping et al. 2017 */
        //from SN
        dustdot += delta_sn * C_sn / dt;
        dustdot += delta_sn * O_sn / dt;
        dustdot += 16 * delta_sn * (Mg_sn/24 + Si_sn/28 + S_sn/32 + Ca_sn/40 + Fe_sn/56) / dt;

        /* eq 6 Popping et al. 2017 */
        //from SNIa
        //printf("dustdot before = %f \n", dustdot);
	//dustdot += 16 * delta_snia * (Fe_snia/56) / dt;
        //dustdot += delta_snia * (Cr_snia + Ni_snia) / dt;

	//printf("dustdot after = %f \n", dustdot);
      	galaxies[p].ColdDust += dustdot * dt;
	galaxies[p].MetalsColdGas -= dustdot * dt;
        galaxies[p].dustdotform[step] += dustdot;
        XPRINT(dustdot * dt >= 0, "dust mass = %.3e, delta dust = %.3e, galaxy id = %i \n", galaxies[p].ColdDust, dustdot*dt, galaxies[p].GalaxyNr);

   }
}

void accrete_dust(const double metallicity, const double dt, const int p, const int step, struct GALAXY *galaxies) {
//dust accretion in ISM : eq 20 Asano13
  double dustdot = 0;
  double tacc_zero = 20 * SEC_PER_MEGAYEAR / run_params.UnitTime_in_s; //should be free parameter! yr

  if (galaxies[p].MetalsColdGas > galaxies[p].ColdDust && metallicity > 0  ) {
    double tacc = tacc_zero * 0.02 / metallicity;
    dustdot += (1 - galaxies[p].ColdDust/galaxies[p].MetalsColdGas) * (galaxies[p].f_H2 * galaxies[p].ColdDust / tacc);
  
    galaxies[p].dustdotgrowth[step] += dustdot;
    galaxies[p].ColdDust += dustdot * dt;
    galaxies[p].MetalsColdGas -= dustdot * dt;
    XPRINT(galaxies[p].ColdDust >= 0, "dust mass = %.3e, delta dust = %.3e, galaxy id = %i \n", galaxies[p].ColdDust, dustdot*dt, galaxies[p].GalaxyNr);
  }

}

void destruct_dust(const double metallicity, const double stars, const double dt, const int p, const int step,  struct GALAXY *galaxies) {
//dust destruction : Asano et al. 13
  int i;
  double sfr;
  double dustdot = 0;
  double age[SNAPLEN], sfh[SNAPLEN];
  double m_low = 8; //lower limit of stellar mass that end up as SN
  double up_binary = 16; //upper limit for binary stars, Msun
  double m_up = 40; //upper limit of stellar mass that end up as SN
  double t_low = compute_taum(m_up); //the shortest stellar lifetime that end up as SN
  double eta = 0.1; //should be free parameter, we used value adopted by Asano et al. 13
  double m_swept = 1535 * pow((metallicity/0.02 + 0.039), -0.289) * run_params.Hubble_h / 1.e10; //eq. 14 Asano et al. 13
  int count = 20;
  double mass[count], phi[count], mphi[count];
  double A = run_params.BinaryFraction;
//  double m_swept = 600 * run_params.Hubble_h / 1.e10;

  for (i=0; i<SNAPLEN; i++)
  {
        age[i] = run_params.lbtime[i]; //change the variable name to represent the age of universe instead of lbtime
        sfh[i] = galaxies[p].Sfr[i];
  }

//Dwek & Cherchneff 2011
  if (age[galaxies[p].SnapNum] > t_low) {
    for(i=0; i<count; i++) {
        mass[i] = 1 + ((m_up - 1) / (count-i));
        phi[i] = compute_imf(mass[i]);
        mphi[i] = mass[i]*phi[i];
    }
/*
    for (i=0; i<count; i++){
	mass[i] = m_low + ((m_up - m_low) / (count-i));
	phi[i] = compute_imf(mass[i]);
        double taum = compute_taum(mass[i]);
        double time = age[galaxies[p].SnapNum] - taum;
        if(time < run_params.lbtime[0]){
                sfr=0;
        }
        else {
                sfr = interpolate_arr(age, sfh, SNAPLEN, time);
        }
	mphi[i] = sfr*phi[i];
    }
*/
  
    double mstar = integrate_arr(mass, mphi, count, mass[0], m_up) / integrate_arr(mass, phi, count, m_low, m_up);
    double mstar_ia = integrate_arr(mass, mphi, count, mass[0], m_up) / integrate_arr(mass, phi, count, m_low, up_binary);
    double mstar_ii = integrate_arr(mass, mphi, count, mass[0], m_up) / integrate_arr(mass, phi, count, up_binary, m_up);
    double Rsn_ia = A*stars / dt / mstar_ia;
    double Rsn_ii = (1-A) * stars / dt / mstar_ia + stars/dt/mstar_ii;
    //printf("%f \n", Rsn_ii/Rsn_ia); 
    double Rsn = stars / dt / mstar;
    assert(m_swept > 0 && "mass of ISM swept by SN must be greater than 0");

//    if (Rsn > 0 && galaxies[p].ColdGas > 0 && galaxies[p].f_HI >0) {
    if (Rsn > 0 && galaxies[p].ColdGas > 0) {
       Rsn *=  run_params.UnitMass_in_g / run_params.UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS; //convert to Msun/yr 
//       double tsn = galaxies[p].ColdGas * galaxies[p].f_HI/ (eta * m_swept * Rsn); //eq.12 Asano et al 13 [yr]
       double tsn = galaxies[p].ColdGas / (eta * m_swept * Rsn); //eq.12 Asano et al 13 [yr]
       tsn /= run_params.UnitTime_in_s / SEC_PER_YEAR; //back to internal unit
//       printf("coldgas = %.3e, dust = %.3e, m_swept = %.3e, Rsn = %.3e,  tsn = %.3e, dt = %.3e Myr \n", galaxies[p].ColdGas, galaxies[p].ColdDust,  m_swept, Rsn, tsn * run_params.UnitTime_in_s / SEC_PER_MEGAYEAR, dt * run_params.UnitTime_in_s / SEC_PER_MEGAYEAR);
//       assert(tsn > 0 && "tsn must be greater than 0");
       dustdot += galaxies[p].ColdDust / tsn;
    }

    if (galaxies[p].ColdDust - dustdot * dt > 0) {
  	  galaxies[p].dustdotdestruct[step] += dustdot;
  	  galaxies[p].ColdDust -= dustdot * dt;
  	  galaxies[p].MetalsColdGas += dustdot * dt;
    }
    else {
	galaxies[p].dustdotdestruct[step] += galaxies[p].ColdDust / dt;
	galaxies[p].MetalsColdGas += galaxies[p].ColdDust;
	galaxies[p].ColdDust = 0;
    }

  }
}


void dust_thermal_sputtering(const int gal, const double dt, struct GALAXY *galaxies) //section 3.5 Popping 2017
{
   double adot;
   double a = 1e-5 / run_params.UnitLength_in_cm; //a = 0.1 micrometer
   double mdot = 0.0;

// thermal sputtering in hot dust
   if(galaxies[gal].HotDust > 0.0 && galaxies[gal].Vvir > 0.0) {
        const double temp0 = 2e6; //in Kelvin
        const double temp = 35.9 * galaxies[gal].Vvir * galaxies[gal].Vvir; //in Kelvin 
        const double gamma = 2.5;

        double x = -3.2e-18 / PROTONMASS / (pow(temp0/temp, gamma) + 1); //in cm^4 / g /s
        x /= run_params.UnitLength_in_cm / run_params.UnitDensity_in_cgs / run_params.UnitTime_in_s; //in internal unit
        double rho = galaxies[gal].HotGas / (4 / 3 * M_PI * galaxies[gal].Rvir * galaxies[gal].Rvir * galaxies[gal].Rvir);
        adot = rho / x;
        a -= adot * dt;

        assert(a > 0);
        a /= 1e-5 / run_params.UnitLength_in_cm; //in 0.1 micrometer unit
        rho *= run_params.UnitDensity_in_cgs;
        const double tau0 = 170 * SEC_PER_MEGAYEAR / run_params.UnitTime_in_s; // 0.17Gyr to internal unit
        const double tau = tau0 * (a/rho) * (pow(temp0/temp, gamma) + 1);
        mdot = galaxies[gal].HotDust / tau / 3;

        galaxies[gal].HotDust -= mdot * dt;
        galaxies[gal].MetalsHotGas += mdot * dt;
   }

// thermal sputtering in ejected dust
   if(galaxies[gal].EjectedDust > 0.0 && galaxies[gal].Vvir > 0.0) {
        const double temp0 = 2e6; //in Kelvin
        const double temp = 35.9 * galaxies[gal].Vvir * galaxies[gal].Vvir; //in Kelvin 
        const double gamma = 2.5;

        double x = -3.2e-18 / PROTONMASS / (pow(temp0/temp, gamma) + 1); //in cm^4 / g /s
        x /= run_params.UnitLength_in_cm / run_params.UnitDensity_in_cgs / run_params.UnitTime_in_s; //in internal unit
        double rho = galaxies[gal].EjectedMass / (4 / 3 * M_PI * galaxies[gal].Rvir * galaxies[gal].Rvir * galaxies[gal].Rvir);
        adot = rho / x;
        a -= adot * dt;

        assert(a > 0);
        a /= 1e-5 / run_params.UnitLength_in_cm; //in 0.1 micrometer unit
        rho *= run_params.UnitDensity_in_cgs;
        const double tau0 = 170 * SEC_PER_MEGAYEAR / run_params.UnitTime_in_s; // 0.17Gyr to internal unit
        const double tau = tau0 * (a/rho) * (pow(temp0/temp, gamma) + 1);
        mdot = galaxies[gal].EjectedDust / tau / 3;

        galaxies[gal].EjectedDust -= mdot * dt;
        galaxies[gal].MetalsEjectedMass += mdot * dt;
   }

}

double integrate_arr(const double arr1[MAX_STRING_LEN], const double arr2[MAX_STRING_LEN], const int npts, const double lower_limit, const double upper_limit)
{
        double Q;
        gsl_interp_accel *acc;
        gsl_spline *spl;

        acc = gsl_interp_accel_alloc ();
        spl = gsl_spline_alloc (gsl_interp_linear, npts);

        gsl_spline_init (spl, arr1, arr2, npts);
        Q = gsl_spline_eval_integ (spl, lower_limit, upper_limit, acc);

        gsl_spline_free (spl);
        gsl_interp_accel_free (acc);

        return Q;
}

double interpolate_arr(const double arr1[MAX_STRING_LEN], const double arr2[MAX_STRING_LEN], const int npts, const int xi)
{
        double Q;
        gsl_interp_accel *acc;
        gsl_spline *spl;

        acc = gsl_interp_accel_alloc ();
        spl = gsl_spline_alloc (gsl_interp_linear, npts);

        gsl_spline_init (spl, arr1, arr2, npts);
        Q = gsl_spline_eval (spl, xi, acc);

        gsl_spline_free (spl);
        gsl_interp_accel_free (acc);
        return Q;
}

double compute_imf (const double m)
{ //eq 11 Arrigoni et al. 2010
        double mass = m;
        double A = 0.9098, B = 0.2539, x = 1.3, sigma=0.69;
        double mc = 0.079; //Msun
        double phi;

        if (m < 1){
                phi = A * exp( -(log10(mass) - log10(mc)) / (2*sigma*sigma));
                }
        else {
                phi = B * pow(mass, -x);
                }

        return phi;
}

double compute_taum (const double m)
{ //eq 3 Raiteri et al. 1996 
        double mass = m;
        double Z = 0.02;
        double a0 = 10.13 + 0.07547*log10(Z) - 0.008084*log10(Z)*log10(Z);
        double a1 = -4.424 - 0.7939*log10(Z) - 0.1187*log10(Z)*log10(Z);
        double a2 = 1.262 + 0.3385*log10(Z) + 0.05417*log10(Z)*log10(Z);

        double logt = a0 + a1*log10(mass) + a2*log10(mass)*log10(mass);
        double t = pow(10, logt) / 1e6  ; //in Myr/h

        /* convert into Myrs/h */
        return t;
}
