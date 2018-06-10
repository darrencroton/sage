#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_integration.h>

#include "core_allvars.h"
#include "core_init.h"
#include "core_mymalloc.h"
#include "core_cool_func.h"


/* These functions do not need to be exposed externally */
double integrand_time_to_present(const double a, void *param);
void set_units(void);
void read_snap_list(void);
double time_to_present(const double z);

#ifdef HDF5
#include "io/io_save_hdf5.h"
#endif

void init(void)
{
    run_params.Age = mymalloc(ABSOLUTEMAXSNAPS*sizeof(*(run_params.Age)));
  
    random_generator = gsl_rng_alloc(gsl_rng_ranlxd1);
    gsl_rng_set(random_generator, 42);	 // start-up seed 

    set_units();

    read_snap_list();

    //Hack to fix deltaT for snapshot 0
    //This way, galsnapnum = -1 will not segfault.
    run_params.Age[0] = time_to_present(1000.0);//lookback time from z=1000
    run_params.Age++;
  
    for(int i = 0; i < run_params.Snaplistlen; i++) {
        run_params.ZZ[i] = 1 / run_params.AA[i] - 1;
        run_params.Age[i] = time_to_present(run_params.ZZ[i]);
    }

    run_params.a0 = 1.0 / (1.0 + run_params.Reionization_z0);
    run_params.ar = 1.0 / (1.0 + run_params.Reionization_zr);

    read_cooling_functions();

#if 0    
#ifdef HDF5
    if(HDF5Output) {
        calc_hdf5_props();
    }
#endif
#endif

}



void set_units(void)
{

    run_params.UnitTime_in_s = run_params.UnitLength_in_cm / run_params.UnitVelocity_in_cm_per_s;
    run_params.UnitTime_in_Megayears = run_params.UnitTime_in_s / SEC_PER_MEGAYEAR;
    run_params.G = GRAVITY / pow(run_params.UnitLength_in_cm, 3) * run_params.UnitMass_in_g * pow(run_params.UnitTime_in_s, 2);
    run_params.UnitDensity_in_cgs = run_params.UnitMass_in_g / pow(run_params.UnitLength_in_cm, 3);
    run_params.UnitPressure_in_cgs = run_params.UnitMass_in_g / run_params.UnitLength_in_cm / pow(run_params.UnitTime_in_s, 2);
    run_params.UnitCoolingRate_in_cgs = run_params.UnitPressure_in_cgs / run_params.UnitTime_in_s;
    run_params.UnitEnergy_in_cgs = run_params.UnitMass_in_g * pow(run_params.UnitLength_in_cm, 2) / pow(run_params.UnitTime_in_s, 2);

    run_params.EnergySNcode = run_params.EnergySN / run_params.UnitEnergy_in_cgs * run_params.Hubble_h;
    run_params.EtaSNcode = run_params.EtaSN * (run_params.UnitMass_in_g / SOLAR_MASS) / run_params.Hubble_h;

    // convert some physical input parameters to internal units 
    run_params.Hubble = HUBBLE * run_params.UnitTime_in_s;

    // compute a few quantitites 
    run_params.RhoCrit = 3.0 * run_params.Hubble * run_params.Hubble / (8 * M_PI * run_params.G);
}



void read_snap_list(void)
{
    char fname[MAX_STRING_LEN+1];

    snprintf(fname, MAX_STRING_LEN, "%s", run_params.FileWithSnapList);
    FILE *fd = fopen(fname, "r"); 
    if(fd == NULL) {
        printf("can't read output list in file '%s'\n", fname);
        ABORT(0);
    }

    run_params.Snaplistlen = 0;
    do {
        if(fscanf(fd, " %lg ", &(run_params.AA[run_params.Snaplistlen])) == 1) {
            run_params.Snaplistlen++;
        } else {
            break;
        }
    } while(run_params.Snaplistlen < run_params.MAXSNAPS);
    fclose(fd);

#ifdef MPI
    if(ThisTask == 0)
#endif
        printf("found %d defined times in snaplist\n", run_params.Snaplistlen);
}



double time_to_present(const double z)
{
#define WORKSIZE 1000
    gsl_function F;
    gsl_integration_workspace *workspace;
    double time, result, abserr;

    workspace = gsl_integration_workspace_alloc(WORKSIZE);
    F.function = &integrand_time_to_present;

    gsl_integration_qag(&F, 1.0 / (z + 1), 1.0, 1.0 / run_params.Hubble,
                        1.0e-8, WORKSIZE, GSL_INTEG_GAUSS21, workspace, &result, &abserr);

    time = 1.0 / run_params.Hubble * result;
    
    gsl_integration_workspace_free(workspace);

    // return time to present as a function of redshift 
    return time;
#undef WORKSIZE    
}



double integrand_time_to_present(const double a, void *param)
{
    (void) param;
    return 1.0 / sqrt(run_params.Omega / a + (1.0 - run_params.Omega - run_params.OmegaLambda) + run_params.OmegaLambda * a * a);
}



