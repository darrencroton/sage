#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"

#include "model_misc.h"

void init_galaxy(const int p, const int halonr, int *galaxycounter, struct halo_data *halos,
                 struct GALAXY *galaxies)
{
  
	assert(halonr == halos[halonr].FirstHaloInFOFgroup);
    
    galaxies[p].Type = 0;
    
    galaxies[p].GalaxyNr = *galaxycounter;
    (*galaxycounter)++;
    
    galaxies[p].HaloNr = halonr;
    galaxies[p].MostBoundID = halos[halonr].MostBoundID;
    galaxies[p].SnapNum = halos[halonr].SnapNum - 1;
    
    galaxies[p].mergeType = 0;
    galaxies[p].mergeIntoID = -1;
    galaxies[p].mergeIntoSnapNum = -1;
    galaxies[p].dT = -1.0;
    
    for(int j = 0; j < 3; j++) {
        galaxies[p].Pos[j] = halos[halonr].Pos[j];
        galaxies[p].Vel[j] = halos[halonr].Vel[j];
    }

    galaxies[p].Len = halos[halonr].Len;
    galaxies[p].Vmax = halos[halonr].Vmax;
    galaxies[p].Vvir = get_virial_velocity(halonr, halos);
    galaxies[p].Mvir = get_virial_mass(halonr, halos);
    galaxies[p].Rvir = get_virial_radius(halonr, halos);
    
    galaxies[p].deltaMvir = 0.0;
    
    galaxies[p].ColdGas = 0.0;
    galaxies[p].StellarMass = 0.0;
    galaxies[p].BulgeMass = 0.0;
    galaxies[p].HotGas = 0.0;
    galaxies[p].EjectedMass = 0.0;
    galaxies[p].BlackHoleMass = 0.0;
    galaxies[p].ICS = 0.0;
    
    galaxies[p].MetalsColdGas = 0.0;
    galaxies[p].MetalsStellarMass = 0.0;
    galaxies[p].MetalsBulgeMass = 0.0;
    galaxies[p].MetalsHotGas = 0.0;
    galaxies[p].MetalsEjectedMass = 0.0;
    galaxies[p].MetalsICS = 0.0;
    
    for(int step = 0; step < STEPS; step++) {
        galaxies[p].SfrDisk[step] = 0.0;
        galaxies[p].SfrBulge[step] = 0.0;
        galaxies[p].SfrDiskColdGas[step] = 0.0;
        galaxies[p].SfrDiskColdGasMetals[step] = 0.0;
        galaxies[p].SfrBulgeColdGas[step] = 0.0;
        galaxies[p].SfrBulgeColdGasMetals[step] = 0.0;
    }

    galaxies[p].DiskScaleRadius = get_disk_radius(halonr, p, halos, galaxies);
    galaxies[p].MergTime = 999.9;
    galaxies[p].Cooling = 0.0;
    galaxies[p].Heating = 0.0;
    galaxies[p].r_heat = 0.0;
    galaxies[p].QuasarModeBHaccretionMass = 0.0;
    galaxies[p].TimeOfLastMajorMerger = -1.0;
    galaxies[p].TimeOfLastMinorMerger = -1.0;
    galaxies[p].OutflowRate = 0.0;
	galaxies[p].TotalSatelliteBaryons = 0.0;
    
	// infall properties
    galaxies[p].infallMvir = -1.0;  
    galaxies[p].infallVvir = -1.0;
    galaxies[p].infallVmax = -1.0;

}



double get_disk_radius(const int halonr, const int p, struct halo_data *halos, struct GALAXY *galaxies)
{
	if(galaxies[p].Vvir > 0.0 && galaxies[p].Rvir > 0.0) {
		// See Mo, Shude & White (1998) eq12, and using a Bullock style lambda.
		double SpinMagnitude = sqrt(halos[halonr].Spin[0] * halos[halonr].Spin[0] + 
                                    halos[halonr].Spin[1] * halos[halonr].Spin[1] + halos[halonr].Spin[2] * halos[halonr].Spin[2]);
        
		double SpinParameter = SpinMagnitude / ( 1.414 * galaxies[p].Vvir * galaxies[p].Rvir);
		return (SpinParameter / 1.414 ) * galaxies[p].Rvir;
        /* return SpinMagnitude * 0.5 / galaxies[p].Vvir; /\* should be equivalent to previous call *\/ */
	} else {
		return 0.1 * galaxies[p].Rvir;
    }
}



double get_metallicity(const double gas, const double metals)
{
  double metallicity = 0.0;

  if(gas > 0.0 && metals > 0.0) {
      metallicity = metals / gas;
      metallicity = metallicity >= 1.0 ? 1.0:metallicity;
  }

  return metallicity;
}



double dmax(const double x, const double y)
{
    return (x > y) ? x:y;
}



double get_virial_mass(const int halonr, struct halo_data *halos)
{
  if(halonr == halos[halonr].FirstHaloInFOFgroup && halos[halonr].Mvir >= 0.0)
    return halos[halonr].Mvir;   /* take spherical overdensity mass estimate */ 
  else
    return halos[halonr].Len * run_params.PartMass;
}



double get_virial_velocity(const int halonr, struct halo_data *halos)
{
	double Rvir;
	
	Rvir = get_virial_radius(halonr, halos);
	
    if(Rvir > 0.0)
		return sqrt(run_params.G * get_virial_mass(halonr, halos) / Rvir);
	else
		return 0.0;
}


double get_virial_radius(const int halonr, struct halo_data *halos)
{
  // return halos[halonr].Rvir;  // Used for Bolshoi
  const int snapnum = halos[halonr].SnapNum;
  const double zplus1 = 1.0 + run_params.ZZ[snapnum];
  const double hubble_of_z_sq =
      run_params.Hubble * run_params.Hubble *(run_params.Omega * zplus1 * zplus1 * zplus1 + (1.0 - run_params.Omega - run_params.OmegaLambda) * zplus1 * zplus1 +
                                              run_params.OmegaLambda);
  
  const double rhocrit = 3.0 * hubble_of_z_sq / (8.0 * M_PI * run_params.G);
  const double fac = 1.0 / (200.0 * 4.0 * M_PI / 3.0 * rhocrit);
  
  return cbrt(get_virial_mass(halonr, halos) * fac);
}



