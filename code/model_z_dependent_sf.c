#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_integration.h>



double metallicity_dependent_star_formation(int p)
{
  double SFR, q_transition, q_crit;
  double metallicity_in_solar, Chi, Xi, Sigma_c0;

  // based on the model of Krumholz & Dekel 2011
  
  metallicity_in_solar = get_metallicity(Gal[p].ColdGas, Gal[p].MetalsColdGas) / 0.02;
  Chi = 0.7561 * (1.0 + pow(metallicity_in_solar, 0.365));
  Xi = log(1.0 + 0.6*Chi + 0.01*Chi*Chi) / (192.0 * metallicity_in_solar * ClumpingFactor);
  Sigma_c0 = Gal[p].ColdGas / (2.0 * M_PI * Gal[p].DiskScaleRadius * Gal[p].DiskScaleRadius) 
    * (UnitMass_in_g / (UnitLength_in_cm * UnitLength_in_cm)); // in g/cm^2
  
  SFR = 0.0; q_crit = 0.0; q_transition = 0.0;

  // note that q = r/scale_radius
  q_crit = log(2.0 * Sigma_c0 / Xi);  // f_H2=0 when r>r_crit, evaluate integral otherwise
  q_transition = log(Sigma_c0 / 0.18);  // epsilon_ff/t_ff transition at Sigma_0=0.18, gamma=1 when r<r_trans, gamma=-1 otherwise
  
  if(q_crit > 0.0)
  {
    if(q_transition > 0.0)
    {
      if(q_crit < q_transition)
        SFR += Z_dependent_SF(0.0, q_crit, Sigma_c0, Xi, 1.0);
      else
      {
        SFR += Z_dependent_SF(0.0, q_transition, Sigma_c0, Xi, 1.0);
        SFR += Z_dependent_SF(q_transition, q_crit, Sigma_c0, Xi, -1.0);              
      }
    }
    else
      SFR += Z_dependent_SF(0.0, q_crit, Sigma_c0, Xi, -1.0);
  }
  
  // if(Gal[p].Mvir > 100.0 && Gal[p].Mvir < 200.0 && Gal[p].Type == 0 && Gal[p].SnapNum == 62) 
  //   printf("%e\t%e\t%e\t%e\t%e\t\t%f\t%f\n", metallicity_in_solar, Chi, Xi, Sigma_c0, SFR, q_crit, q_transition); fflush(stdout);  
  
  return (Gal[p].ColdGas * 1.0e10) / (2.6 * 1.0e9) * SFR / (UnitMass_in_g / UnitTime_in_s * SEC_PER_YEAR / SOLAR_MASS);

}



struct F_parameters { double a; double b; double c; };

double Z_dependent_SF(float lower_limit, float upper_limit, float Sigma_c0, float Xi, float gamma)
{
#define WORKSIZE 512
  static gsl_function F;
  static gsl_integration_workspace *workspace;
  double result, abserr;

  struct F_parameters parameters = { Sigma_c0, Xi, gamma };
  workspace = gsl_integration_workspace_alloc(WORKSIZE);
  
  F.function = &integrand_Z_dependent_SF;
  F.params = &parameters;
  
  gsl_integration_qag(&F, lower_limit, upper_limit, 1.0e-8, 1.0e-8, WORKSIZE, GSL_INTEG_GAUSS15, workspace, &result, &abserr);
  gsl_integration_workspace_free(workspace);
  
  return result;
}



double integrand_Z_dependent_SF(double q, void *p)
{
  struct F_parameters * params = (struct F_parameters *)p;

  double Sigma_c0 = (params->a);
  double Xi = (params->b);
  double gamma = (params->c);
  
  return (1.0 - 0.75 / ((Sigma_c0/Xi) * exp(-q) + 0.25)) * pow(Sigma_c0 / 0.18, 0.33*gamma) * pow(exp(-q), 0.33*gamma+1.0) * q;
}


