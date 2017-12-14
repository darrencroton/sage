#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"



double cooling_recipe(int gal, int centralgal, double dt, double time)
{
  double tcool, x, logZ, lambda, rcool, rho_rcool, rho0, temp, coolingGas, rho_cool;

  if(Gal[gal].HotGas > 0.0 && Gal[gal].Vvir > 0.0)
  {
    tcool = Gal[gal].Rvir / Gal[gal].Vvir;
    temp = RadioLuminosity_jet(gal, centralgal, time, dt);         // in Kelvin

    if(Gal[gal].MetalsHotGas > 0)
      logZ = log10(Gal[gal].MetalsHotGas / Gal[gal].HotGas);
    else
      logZ = -10.0;

    lambda = get_metaldependent_cooling_rate(log10(temp), logZ);
    x = PROTONMASS * BOLTZMANN * temp / lambda;        // now this has units sec g/cm^3  
    x /= (UnitDensity_in_cgs * UnitTime_in_s);         // now in internal units 
    rho_rcool = x / tcool * 0.885;  // 0.885 = 3/2 * mu, mu=0.59 for a fully ionized gas

    // An isothermal density profile for the hot gas is assumed here
    if(Density_model == 0)
    {
      rho0 = Gal[gal].HotGas / (4 * M_PI * Gal[gal].Rvir);
      rcool = sqrt(rho0 / rho_rcool);
        
      if(rcool > Gal[gal].Rvir)
        // "cold accretion" regime
        coolingGas = Gal[gal].HotGas / (Gal[gal].Rvir / Gal[gal].Vvir) * dt;
      else if (rcool > 0.0)
        // "hot halo cooling" regime
        coolingGas = (Gal[gal].HotGas / Gal[gal].Rvir) * (rcool / (2.0 * tcool)) * dt;
      else
        coolingGas = 0.0;
    
    if(coolingGas > Gal[gal].HotGas)
      coolingGas = Gal[gal].HotGas;
    else
      if(coolingGas < 0.0)
        coolingGas = 0.0;
    }

    // density profile of Makino et al. (1998) for the hot gas is assumed here
    if(Density_model == 1)
    {
      double A_b,D_ratio, beta_eff, Rc_eff;
      rho0 = density_profile (gal);
      A_b = -0.178 *Gal[gal].b_gas + 0.982;
      beta_eff =  0.9*Gal[gal].b_gas;
      Rc_eff   = 0.22* Gal[gal].Rs;
      D_ratio = pow(A_b * rho0/rho_rcool, 2.0/(3* beta_eff));
      rcool = (Rc_eff) * pow(D_ratio -1, 0.5);

      if(rcool > Gal[gal].Rvir)
        // "cold accretion" regime
        coolingGas = Gal[gal].HotGas / (Gal[gal].Rvir / Gal[gal].Vvir) * dt;
      else if(rcool > 0.0)
        // "Hot halo cooling" regime base on some calculation on dr/dt according to t_cool relation
        coolingGas = (4.0 * M_PI * rho0 * pow(rcool,3)/tcool) * (rcool/Gal[gal].Rs) * exp(-13.5*Gal[gal].b_gas) * pow(1+(rcool/Gal[gal].Rs),13.5*Gal[gal].b_gas/(rcool/Gal[gal].Rs)) /(13.5*Gal[gal].b_gas*(log(1+(rcool/Gal[gal].Rs))-(rcool/(rcool+Gal[gal].Rs)))) * dt;
      else
        coolingGas = 0.0;
        
      if(coolingGas > Gal[gal].HotGas)
        coolingGas = Gal[gal].HotGas;
      else
        if(coolingGas < 0.0)
          coolingGas = 0.0;
    }
    
    // Uplifting gas for shocked radius. There is'nt cooling in the quiescence time scale of AGN activity
    if (AGN_model == 1 )
    {
      if(AGNrecipeOn > 0 && coolingGas > 0.0)
        coolingGas = do_Jet_uplift(coolingGas, gal, dt, x, rcool);
    }

    // Estimate cooling time
    rho_cool = rho0 * exp(-13.5*Gal[gal].b_gas) *pow(1+(rcool/Gal[gal].Rs),13.5*Gal[gal].b_gas/(rcool/Gal[gal].Rs));
    Gal[gal].t_cool_Makino = x / rho_cool * 0.885;
    
    // at this point we have calculated the maximal cooling rate
    // if AGNrecipeOn we now reduce it in line with past heating before proceeding

    if(AGN_model == 0)
    {
      if(AGNrecipeOn > 0 && coolingGas > 0.0)
        coolingGas = do_AGN_heating(coolingGas, gal, dt, x, rcool);
    }

    if (coolingGas > 0.0)
      Gal[gal].Cooling += 0.5 * coolingGas * Gal[gal].Vvir * Gal[gal].Vvir;
      // White & Frenk (1991)
      Gal[gal].Lx_bol  +=  2.5 * coolingGas * Gal[gal].Vvir * Gal[gal].Vvir;
    }
	else
		coolingGas = 0.0;

	assert(coolingGas >= 0.0);
  return coolingGas;

}



double do_AGN_heating(double coolingGas, int centralgal, double dt, double x, double rcool)
{
  double AGNrate, EDDrate, AGNaccreted, AGNcoeff, AGNheating, metallicity, r_heat_new;

	// first update the cooling rate based on the past AGN heating
	if(Gal[centralgal].r_heat < rcool)
		coolingGas = (1.0 - Gal[centralgal].r_heat / rcool) * coolingGas;
	else
		coolingGas = 0.0;
	
	assert(coolingGas >= 0.0);

	// now calculate the new heating rate
  if(Gal[centralgal].HotGas > 0.0)
  {

    if(AGNrecipeOn == 2)
    {
      // Bondi-Hoyle accretion recipe
      AGNrate = (2.5 * M_PI * G) * (0.375 * 0.6 * x) * Gal[centralgal].BlackHoleMass * RadioModeEfficiency;
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
      if(Gal[centralgal].Mvir > 0.0)
        AGNrate = RadioModeEfficiency / (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS)
          * (Gal[centralgal].BlackHoleMass / 0.01) * pow(Gal[centralgal].Vvir / 200.0, 3.0)
            * ((Gal[centralgal].HotGas / Gal[centralgal].Mvir) / 0.1);
      else
        AGNrate = RadioModeEfficiency / (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS)
          * (Gal[centralgal].BlackHoleMass / 0.01) * pow(Gal[centralgal].Vvir / 200.0, 3.0);
    }
    
    // Eddington rate 
    EDDrate = (1.3e38 * Gal[centralgal].BlackHoleMass * 1e10 / Hubble_h) / (UnitEnergy_in_cgs / UnitTime_in_s) / (0.1 * 9e10);

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

    /// the above is the maximal heating rate. we now limit it to the current cooling rate
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
  
		// update the heating radius as needed
		if(Gal[centralgal].r_heat < rcool && coolingGas > 0.0)
		{
			r_heat_new = (AGNheating / coolingGas) * rcool;
			if(r_heat_new > Gal[centralgal].r_heat)
				Gal[centralgal].r_heat = r_heat_new;
		}
		
		if (AGNheating > 0.0)
			Gal[centralgal].Heating += 0.5 * AGNheating * Gal[centralgal].Vvir * Gal[centralgal].Vvir;
  }

  return coolingGas;

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


// radio AGN jet model
double do_Jet_uplift(double coolingGas, int p, double dt, double x, double rcool)
{
    // AGN active time
    double AGN_Active_time, C_s,Mcool,rho0,R_0;
    AGN_Active_time = Gal[p].t_AGN_on;

    //        Makino (1998) density
    rho0 = density_profile (p);
    //  Duty cycle  
    Gal[p].delta = Co_delta * pow(Gal[p].StellarMass * 1e10/1e11,1.5);
    
    if (Gal[p].delta > 1)
        Gal[p].delta = 1.0;
    
    //  Time after t_on until start of next AGN episod
    Gal[p].t_AGN_off = AGN_Active_time * ((1.0/Gal[p].delta) - 1.0);
    
    // sound speed cs = [gama KB Tvir/(mu mH)]^0.5 ~ 0.91 Vvir
    C_s = pow(5.0/3.0 * 0.5 * Gal[p].Vvir*Gal[p].Vvir,0.5);
    
    // Time for gas to return from r = Rshocked to r = 0
    Gal[p].t_AGN_return =  Co_return *((Gal[p].Rshocked/Hubble_h) / Gal[p].Vvir) * UnitTime_in_Megayears;
    
    // Time of next AGN on
    Gal[p].time_to_next_on = Gal[p].t_AGN_off + AGN_Active_time;
    
    // Time from return to the next on
    if (Gal[p].t_AGN_off >  Gal[p].t_AGN_return)
        Gal[p].t_static = Gal[p].t_AGN_off - Gal[p].t_AGN_return;
    else
        Gal[p].t_static = 0.0;
    
    // Fraction of cooling gas
    R_0 = 0.22 * Gal[p].Rs;
    Mcool =  16 * M_PI * (rho0) * pow(R_0,3)*(log(1+rcool/R_0)- rcool/(R_0+rcool));

    if (Uplifting == 1 )
    {
        if (rcool > Gal[p].Rshocked )
             Gal[p].fcool    = (Gal[p].t_static/Gal[p].time_to_next_on)+((AGN_Active_time+Gal[p].t_AGN_return)/Gal[p].time_to_next_on)*(Mcool-Gal[p].Mshocked)/Mcool;
        else
             Gal[p].fcool    = (Gal[p].t_static/Gal[p].time_to_next_on);
    }     
    if (Uplifting == 0 )
    {
        if (rcool > Gal[p].Rshocked )
            Gal[p].fcool    = (Gal[p].t_static/Gal[p].time_to_next_on);
    }
   
    // effect of cooling fraction on the cooling gas
    if (Gal[p].fcool > 0.0 && Gal[p].fcool < 1.0)
        coolingGas =  Gal[p].fcool * coolingGas;
    
    assert(coolingGas >= 0.0);
    return coolingGas;
}


double RadioLuminosity_jet(int p, int centralgal, double time, double dt)
{
    double tcool, logZ, lambda, x, temp, AGNrate, EDDrate, rho_0, R_0, a_D, lambda_Rshock, fp, r, uB, V, temp_new, betaparameter, tau, p_v, AGN_Active_time;
    tcool = Gal[p].Rvir / Gal[p].Vvir;
    temp = 35.9 * Gal[p].Vvir * Gal[p].Vvir;         // in Kelvin
    
    if(Gal[p].MetalsHotGas > 0)
    logZ = log10(Gal[p].MetalsHotGas / Gal[p].HotGas);
    else
    logZ = -10.0;
    
    lambda = get_metaldependent_cooling_rate(log10(temp), logZ);
    x = PROTONMASS * BOLTZMANN * temp / lambda;        // now this has units sec g/cm^3
    x /= (UnitDensity_in_cgs * UnitTime_in_s);         // now in internal units
    
    // Bondi-Hoyle accretion recipe * RadioModeEfficiency = 1.0 (see Croton et al. 2016)
    AGNrate = (2.5 * M_PI * G) * (0.375 * 0.6 * x) * Gal[p].BlackHoleMass;
    
    // Eddington rate
    EDDrate = (1.3e38 * Gal[p].BlackHoleMass * 1e10 / Hubble_h) / (UnitEnergy_in_cgs / UnitTime_in_s) / (0.1 * 9e10);
    // accretion onto BH is always limited by the Eddington rate
    if(AGNrate > EDDrate)
    {
        AGNrate = EDDrate;
    }

    Gal[p].RadioAGNaccretionRate = AGNrate * (UnitMass_in_g/SOLAR_MASS/Hubble_h)/(UnitTime_in_s/SEC_PER_MEGAYEAR); // msun/Myr

    // eta
    if(AGNrate < 0.03 * EDDrate)
    {
       Gal[p].Qjet =  eta * AGNrate * 2.9979e8 * 2.9979e8 * (UnitMass_in_g/ Hubble_h/ Hubble_h/ 1e3 / UnitTime_in_s); // [W]
       Gal[p].R_index =1.0;
    }
    else
    {
       Gal[p].Qjet = 6.3e-5 * pow((Gal[p].BlackHoleMass * UnitMass_in_g / SOLAR_MASS/Hubble_h)/ 1e9,-0.1) * pow((AGNrate/EDDrate),0.2) * (AGNrate * pow(C,2) * (UnitMass_in_g/ 1e3/ Hubble_h/ Hubble_h/Hubble_h / UnitTime_in_s));
       Gal[p].Q_index =1.0;
    }
        
    //    Isothermal density profile
    if(Density_model == 0)
    {
        betaparameter = 0.0;
        R_0   = 0.2 * Gal[p].Rvir;
        rho_0 = Gal[p].HotGas / (4.0 * M_PI * Gal[p].Rvir * R_0* R_0) ;
        rho_0 *= 1000.0 * UnitDensity_in_cgs * pow(Hubble_h,3);  // kg m3/h3
    }
    
    //    Makino (1998) Density profile
    if(Density_model == 1)
    {
        rho_0 = density_profile (p);    // unit of system
        rho_0 *= 1000.0 * UnitDensity_in_cgs * pow(Hubble_h,3); // convert to [(kg) / (m3/h3)]
        R_0   =  0.22 * Gal[p].Rs;
        
        // coefficient 3 for defferent between the beta in Shabala et al. (2009)(beta = 3 * beta_eff)
        betaparameter = 3.0 *  0.9 * Gal[p].b_gas;
        // The model just work for beta less than 2
        if (betaparameter > 2 )
            betaparameter = 1.99;
        
    }
  
    // Turner & Shabala et al.  (2015)
    Gal[p].t_AGN_on = Co_ton * pow(Gal[p].StellarMass * 1e10/1e11,0.7);
    AGN_Active_time = Gal[p].t_AGN_on;
    a_D = 1.0;
    tau = pow(pow(R_0 * M_PER_MPC,5) * rho_0  /(Gal[p].Qjet)  ,1.0/3.0)/SEC_PER_MEGAYEAR;
    Gal[p].Rcocoon = a_D * R_0 * pow(((AGN_Active_time)/tau),3/(5 - betaparameter));
    
    // Maximum radius of shocked gas expansion-- Leahy et al. (1989), Subrahmanyan et al. (1996)
    lambda_Rshock = pow(1.0 - (15.0/(4.0*(11.0-betaparameter))), 1.0/3.0);
    Gal[p].Rshocked = (1.0/lambda_Rshock) * Gal[p].Rcocoon;
    
    // Alexander (2002)- Shabala et al. (2013) Temperature of post-shock (mu mH/kB = 0.59 * mH/kB = 71.8 s2/km2)
    Gal[p].Tshocked = (15.0/16.0) * (3.0 - betaparameter)/(11.0 - betaparameter) * (71.8)  * pow((3/(5-betaparameter)) * Gal[p].Rshocked/Hubble_h * ( M_PER_MPC/1000.0 ) /( AGN_Active_time * SEC_PER_MEGAYEAR ),2.0);
    
    // Shock mass in system unit
    Gal[p].Mshocked =  16.0 * M_PI * (rho_0/(1000.0 * UnitDensity_in_cgs * pow(Hubble_h,3))) * pow(R_0,3)*(log(1+Gal[p].Rshocked/R_0)- Gal[p].Rshocked/(R_0+Gal[p].Rshocked));
    // Shocked mass can not be more than hot gas
    if(Gal[p].Mshocked > Gal[p].HotGas)
        Gal[p].Mshocked = Gal[p].HotGas;

    // Radio Luminosity base on Shabala (2013)
    double re = 2.8179403267e-15; // m
    double q = 1.60217662e-19;    // coulombs
    double me = 9.10938356e-31;   // kg
    double mu0 = 1.25663706e-6;   // m kg s-2 A-2  or m kg  Coulomb^-2
    double C_speed = 2.9979e8;    // m/s
    double gam1 = 1.0;   // ? relate to the initial electron energy distribution n(gama) = ke gama_i^ (-p)
    double gam2 = 1e6;   // ? and are minimum and Maximum of Lorentz factor
    double A1;  //1.01236164e-09(p_v = 2.5,gam1 =1.0) or 3.54582781e-08(p_v = 2.5,gam1=1e3)--> kg-1 m s
    
    int ii = 0;
    double c2_p[7];

     for(p_v = 2.1; p_v < 2.8; p_v+=0.1)  //  ? The Fiducial Values
     {
        //    calculate c2_p with http://www.wolframalpha.com --->(3^(x/2)/((2^((x+13)/2))* 3.14^((x+2)/2)))*gama function ((x+1)/4) * gama function (x/4 + 19/12) * gama function (x/4 - 1/12) / gama function ((x+7)/4) for x=p_v
        c2_p[0] =  0.00354432;  //@ p_v = 2.1
        c2_p[1] =  0.00314536;  //@ p_v = 2.2
        c2_p[2] =  0.00280515;  //@ p_v = 2.3
        c2_p[3] =  0.00251322;  //@ p_v = 2.4
        c2_p[4] =  2.26e-3;     //@ p_v = 2.5  alfa = -0.75
        c2_p[5] =  0.00204273;  //@ p_v = 2.6  alfa = -0.8
        c2_p[6] =  0.00185219;  //@ p_v = 2.7  alfa = -0.85
        
        V = M_PI * pow(AxialRatiojet,2) * pow(2.0 * Gal[p].Rshocked * (M_PER_MPC) , 3.0);
        r = (p_v + 1.0)/4.0;
        fp = (18.0 * pow(a_D, 2.0 * (5.0 - betaparameter)/3.0)) / ((gamma_x + 1) * pow(5.0 - betaparameter,2) * pow(AxialRatiojet,2));
        
        // Magnetic field energy density proportional to cocoon Radius  [j/m3] or [kg m-1 s-2]
        // Fraction 0.5 in jet power is due to dividing the jets in 2 direction
        uB =  (r * fp) * pow(rho_0  * pow(R_0 * M_PER_MPC,betaparameter) * pow(0.5 * (Gal[p].Qjet), 2), 1.0/3.0) * pow(2 * Gal[p].Rshocked/Hubble_h * M_PER_MPC  , (-4.0 - betaparameter)/3.0)/((r + 1.0) * (gamma_c - 1.0));

        // Croston et al. 2005 relation between B = 0.7 B_eq
        uB = 0.7 *  uB;
        A1 = (16.0 * M_PI * M_PI * re/C_speed)*pow(q/me, (p_v+1.0)/2.0)*pow(2.0*mu0, (p_v+1.0)/4.0) * (c2_p[ii]/((pow(gam2,2.0-p_v)-pow(gam1,2.0-p_v))/(2.0-p_v)));
        if(AGNrate < 0.03 * EDDrate)
        //  Radio Luminosity[W/Hz]
            Gal[p].RadioLuminosity[ii] = A1 * pow(uB, (5.0 + p_v)/4) * V * pow(1.4e9, (1.0 - p_v)/2.0) * pow(1 + ZZ[Gal[p].SnapNum], (3.0 - p_v)/2.0);
        ii +=1;
     }
    
    // Radio luminosity in Various AGN active time
    int ti; 
    double Rcocoon_lifetime,uB_lifetime;
    for(ti = 0; ti <= Gal[p].t_AGN_on; ti+=1.0)
    {
        p_v = 2.6;
        //lifetime[ti] = ti;
        Rcocoon_lifetime = a_D * R_0 * pow(((ti)/tau),3/(5 - betaparameter));
        Gal[p].Rshocked_lifetime[ti] = (1.0/lambda_Rshock) * Rcocoon_lifetime;
        uB_lifetime = 0.7 * (r * fp) * pow(rho_0  * pow(R_0 * M_PER_MPC,betaparameter) * pow(0.5 * (Gal[p].Qjet), 2), 1.0/3.0) * pow(2 * Gal[p].Rshocked_lifetime[ti]/Hubble_h * M_PER_MPC  , (-4.0 - betaparameter)/3.0)/((r + 1.0) * (gamma_c - 1.0));
        A1 = (16.0 * M_PI * M_PI * re/C_speed)*pow(q/me, (p_v+1.0)/2.0)*pow(2.0*mu0, (p_v+1.0)/4.0) * (c2_p[5]/((pow(gam2,2.0-p_v)-pow(gam1,2.0-p_v))/(2.0-p_v)));
        if(AGNrate < 0.03 * EDDrate)
        //  Radio Luminosity[W/Hz]
        Gal[p].RadioLuminosity_lifetime[ti] = A1 * pow(uB_lifetime, (5.0 + p_v)/4) * V * pow(1.4e9, (1.0 - p_v)/2.0) * pow(1 + ZZ[Gal[p].SnapNum], (3.0 - p_v)/2.0);
    }

    // New hot temperature
    temp_new = (Gal[p].Mshocked/Gal[p].HotGas)*(Gal[p].Tshocked-temp) + temp;         // in Kelvin

    if(AGN_model == 1)
    {
        if (temp_new > temp )
            temp = temp_new;
    }
    Gal[p].Temp_Gas = temp;
    return temp;
}


double density_profile (int p)
{
    double delta_crit, rho_crit, conc, Rvir, Rs, Tvir, delta_nfw, cs, b, rho0, rho0_Makino, rho0_Capelo, zplus1, hubble_of_z_sq;
    //    (Bryan & Norman 1998)
    delta_crit= 18.0 * M_PI * M_PI;
    zplus1 = 1 + ZZ[Gal[p].SnapNum];
    hubble_of_z_sq = Hubble * Hubble *(Omega * zplus1 * zplus1 * zplus1 + (1 - Omega - OmegaLambda) * zplus1 * zplus1 +OmegaLambda);
    rho_crit = 3 * hubble_of_z_sq / (8 * M_PI * G) * UnitDensity_in_cgs/1000.0; // kg/m3

    if(Concentration_Model == 0 )
    {
    //    Bullock et al. (2001) M* = 1.3 * 1e13 M_sun/h
          conc  = (Co_conc/zplus1) * pow(10,((-0.13)*(log10(Gal[p].Mvir * 1e10/Hubble_h) - log10(1.4*1e14/Hubble_h))));
    }
    if(Concentration_Model == 1 )    
    {
    //    Prada et al. 2012 base on Vmax/Vvir    
          conc  = Concentration_halo(p);
    }
    Rvir  = Gal[p].Rvir; // Mpc
    Rs    = Rvir/conc;   // Mpc

    Tvir = (gamma_isothermal / 3 ) * (71.8) * Gal[p].Vvir * Gal[p].Vvir;
    
    delta_nfw=4 * M_PI * (GRAVITY/1e3) * delta_crit * rho_crit * (71.8e-10 / Tvir)* pow(Rs * CM_PER_MPC , 2);
    cs=sqrt(gamma_c * Tvir / 71.8e-10); // from virial temperature
    
    //  gas profile parameters
    //  work out parameter b of Makino+98, at the virial radius; note that
    //  similar number is obtained at R_s
    b = (2 * conc / ( 9 * gamma_isothermal)) / (log(1+conc)-conc/(1+conc));
    
    // Integral on the density profile
    int N = 50;
    double i, fun_Capelo, fun_Makino ,integral_rho0_Makino = 0, integral_rho0_Capelo = 0;
    for (i = 0; i < conc; i += (conc - 0) / N)
    {
        //Capelo+12 profile (WRONG -in progress - you can use another density profile here)
        fun_Capelo = pow(i,2) * pow(1+i, delta_nfw / i);
        integral_rho0_Capelo += fun_Capelo * (conc - 0) / N;
        //Makino profile
        fun_Makino = pow(i,2) * pow(1+i, 13.5 * b / i);
        integral_rho0_Makino += fun_Makino * (conc - 0) / N;
    }
    //  Capelo+12 profile (WRONG -in progress)
    rho0_Capelo=(Gal[p].HotGas)/(4* M_PI * pow(Rs,3) * exp(-delta_nfw)*1e-2)/integral_rho0_Capelo;
    //   Makino profile
    rho0_Makino=(Gal[p].HotGas)*exp(13.5*b)/(4 * M_PI * pow(Rs,3))/integral_rho0_Makino; //system unit

    Gal[p].rho_zero_iso =Gal[p].HotGas / (4 * M_PI * Gal[p].Rvir);
    Gal[p].rho_zero_Capelo = rho0_Capelo * 1000.0 * UnitDensity_in_cgs;
    Gal[p].rho_zero_Makino = rho0_Makino * 1000.0 * UnitDensity_in_cgs;
    Gal[p].b_gas = b;
    Gal[p].Rs = Rs; //Mpc
    Gal[p].concentration = conc;
    
    rho0 = rho0_Makino;
    //rho0 = rho0_Capelo;    
    
    return rho0;
}


// Estimation of halo concentration base on Prada et al. 2012 by using Newton-raphson method
double Concentration_halo (int p)
{
    double conc, v_ratio,x,X0;
    float fx,dfx;
    int itt=100,i=0;
    
    fx=0;
    dfx=0;
    X0 = 5.0;
    
    v_ratio = Gal[p].Vmax/Gal[p].Vvir;
    x= X0;
    for (i = 0; i < itt; i += 1)
    {
      fx = ((0.216 * x)/(log(1+x)-x/(1+x)))-v_ratio;
    
      dfx = (0.216/(log(1+x)-x/(1+x)))-((0.216 * pow(x,2))/(pow(x+1,2)*pow(log(1+x)-x/(1+x),2)));
    
      x = x - fx/dfx;
    }
      conc = x; 
    return conc;
}




