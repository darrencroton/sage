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
#include "core_proto.h"

/* These functions do not need to be exposed externally */
double integrand_time_to_present(const double a, void *param);
void set_units(void);
void read_snap_list(void);
double time_to_present(const double z);


void init(void)
{
    Age = mymalloc(ABSOLUTEMAXSNAPS*sizeof(*Age));
  
    random_generator = gsl_rng_alloc(gsl_rng_ranlxd1);
    gsl_rng_set(random_generator, 42);	 // start-up seed 

    set_units();

    read_snap_list();

    //Hack to fix deltaT for snapshot 0
    //This way, galsnapnum = -1 will not segfault.
    Age[0] = time_to_present(1000.0);//lookback time from z=1000
    Age++;
  
    for(int i = 0; i < Snaplistlen; i++) {
        ZZ[i] = 1 / AA[i] - 1;
        Age[i] = time_to_present(ZZ[i]);
    }

    a0 = 1.0 / (1.0 + Reionization_z0);
    ar = 1.0 / (1.0 + Reionization_zr);

    read_cooling_functions();
}



void set_units(void)
{

    UnitTime_in_s = UnitLength_in_cm / UnitVelocity_in_cm_per_s;
    UnitTime_in_Megayears = UnitTime_in_s / SEC_PER_MEGAYEAR;
    G = GRAVITY / pow(UnitLength_in_cm, 3) * UnitMass_in_g * pow(UnitTime_in_s, 2);
    UnitDensity_in_cgs = UnitMass_in_g / pow(UnitLength_in_cm, 3);
    UnitPressure_in_cgs = UnitMass_in_g / UnitLength_in_cm / pow(UnitTime_in_s, 2);
    UnitCoolingRate_in_cgs = UnitPressure_in_cgs / UnitTime_in_s;
    UnitEnergy_in_cgs = UnitMass_in_g * pow(UnitLength_in_cm, 2) / pow(UnitTime_in_s, 2);

    EnergySNcode = EnergySN / UnitEnergy_in_cgs * Hubble_h;
    EtaSNcode = EtaSN * (UnitMass_in_g / SOLAR_MASS) / Hubble_h;

    // convert some physical input parameters to internal units 
    Hubble = HUBBLE * UnitTime_in_s;

    // compute a few quantitites 
    RhoCrit = 3 * Hubble * Hubble / (8 * M_PI * G);
}



void read_snap_list(void)
{
    char fname[MAX_STRING_LEN+1];

    snprintf(fname, MAX_STRING_LEN, "%s", FileWithSnapList);
    FILE *fd = fopen(fname, "r"); 
    if(fd == NULL) {
        printf("can't read output list in file '%s'\n", fname);
        ABORT(0);
    }

    Snaplistlen = 0;
    do {
        if(fscanf(fd, " %lg ", &AA[Snaplistlen]) == 1) {
            Snaplistlen++;
        } else {
            break;
        }
    } while(Snaplistlen < MAXSNAPS);
    fclose(fd);

#ifdef MPI
    if(ThisTask == 0)
#endif
        printf("found %d defined times in snaplist\n", Snaplistlen);
}



double time_to_present(const double z)
{
#define WORKSIZE 1000
    gsl_function F;
    gsl_integration_workspace *workspace;
    double time, result, abserr;

    workspace = gsl_integration_workspace_alloc(WORKSIZE);
    F.function = &integrand_time_to_present;

    gsl_integration_qag(&F, 1.0 / (z + 1), 1.0, 1.0 / Hubble,
                        1.0e-8, WORKSIZE, GSL_INTEG_GAUSS21, workspace, &result, &abserr);

    time = 1.0 / Hubble * result;
    
    gsl_integration_workspace_free(workspace);

    // return time to present as a function of redshift 
    return time;
#undef WORKSIZE    
}



double integrand_time_to_present(const double a, void *param)
{
    (void) param;
    return 1.0 / sqrt(Omega / a + (1.0 - Omega - OmegaLambda) + OmegaLambda * a * a);
}



