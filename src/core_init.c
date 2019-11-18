#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#ifdef GSL_FOUND
#include <gsl/gsl_integration.h>
#endif

#include "core_allvars.h"
#include "core_init.h"
#include "core_mymalloc.h"
#include "core_cool_func.h"
//#include "core_metal_yield.h"

/* These functions do not need to be exposed externally */
double integrand_time_to_present(const double a, void *param);
void set_units(void);
void read_snap_list(const int ThisTask);
double time_to_present(const double z);
struct table{
        double tbl[1000][17];
        int nr;
};
struct table read_table(char *fname, int ncols);
void read_metal_yield();

#ifdef HDF5
#include "io/io_save_hdf5.h"
#endif

void init(const int ThisTask)
{
    run_params.Age = mymalloc(ABSOLUTEMAXSNAPS*sizeof(*(run_params.Age)));
  
    set_units();

    read_snap_list(ThisTask);

    //Hack to fix deltaT for snapshot 0
    //This way, galsnapnum = -1 will not segfault.
    run_params.Age[0] = time_to_present(1000.0);//lookback time from z=1000
    run_params.Age++;
  
    for(int i = 0; i < run_params.Snaplistlen; i++) {
        run_params.ZZ[i] = 1 / run_params.AA[i] - 1;
        run_params.Age[i] = time_to_present(run_params.ZZ[i]);  //this is actually a lookback time
	run_params.lbtime[i] = (run_params.Age[0] - run_params.Age[i]) * run_params.UnitTime_in_s / SEC_PER_MEGAYEAR; //age of universe since bigbang
    }

    run_params.a0 = 1.0 / (1.0 + run_params.Reionization_z0);
    run_params.ar = 1.0 / (1.0 + run_params.Reionization_zr);

    read_cooling_functions();
    if(ThisTask == 0) {
        printf("cooling functions read\n");
    }

    read_metal_yield();
    if(ThisTask == 0){
	printf("metal yields read\n\n");
    }

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



void read_snap_list(const int ThisTask)
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

    if(ThisTask == 0) {
        printf("found %d defined times in snaplist\n", run_params.Snaplistlen);
    }
}



double time_to_present(const double z)
{
    const double end_limit = 1.0;
    const double start_limit = 1.0/(1 + z);
    double result=0.0;
#ifdef GSL_FOUND
#define WORKSIZE 1000
    gsl_function F;
    gsl_integration_workspace *workspace;
    double abserr;

    workspace = gsl_integration_workspace_alloc(WORKSIZE);
    F.function = &integrand_time_to_present;

    gsl_integration_qag(&F, start_limit, end_limit, 1.0 / run_params.Hubble,
                        1.0e-9, WORKSIZE, GSL_INTEG_GAUSS21, workspace, &result, &abserr);

    gsl_integration_workspace_free(workspace);

#undef WORKSIZE    
#else
    /* Do not have GSL - let's integrate numerically ourselves */
    const double step  = 1e-7;
    const int64_t nsteps = (end_limit - start_limit)/step;
    result = 0.0;
    const double y0 = integrand_time_to_present(start_limit + 0*step, NULL);
    const double yn = integrand_time_to_present(start_limit + nsteps*step, NULL);
    for(int64_t i=1; i<nsteps; i++) {
        result  += integrand_time_to_present(start_limit + i*step, NULL);
    }

    result = (step*0.5)*(y0 + yn + 2.0*result);
#endif

    /* convert into Myrs/h (I think -> MS 23/6/2018) */
    const double time = 1.0 / run_params.Hubble * result;

    // return time to present as a function of redshift 
    return time;
}



double integrand_time_to_present(const double a, void *param)
{
    (void) param;
    return 1.0 / sqrt(run_params.Omega / a + (1.0 - run_params.Omega - run_params.OmegaLambda) + run_params.OmegaLambda * a * a);
}


void read_metal_yield(void)
{
        char fname[MAX_STRING_LEN];
	int cols=0;
	int i, j, rows;

	/* READ AGB YIELD */
	if (run_params.AGBYields == 0)
	{
		double Z_std[7] = {0.0, 1e-4, 4e-4, 4e-3, 8e-3, 0.02, 0.05}; //metal grid from Karakas 10
		strcpy(fname, "src/auxdata/yields/table2d.dat");
		cols = 13; //number of columns in the file
	

        	struct table data;
        	data = read_table(fname, cols);
        	rows = data.nr;
		double Z[rows];
        	
        	//count how many rows necessary for each column based on Z
        	int count = 0;
        	for (i=0; i<rows; i++)
        	{       
                	Z[i] = data.tbl[i][0];
                	if (Z[i] == Z_std[0]) {
                        	count++;
			}
        	}
        
        	//now assign
		run_params.countagb = count;
        
        	for (j=0; j<7; j++){
                	int index = 0;
                	for (i=0; i<rows; i++){
                        	if (Z[i] == Z_std[j])
                        	{       
                                	run_params.magb[index] = data.tbl[i][1]; //1 is the column number
                                	run_params.qCagb[index][j] = data.tbl[i][6] + data.tbl[i][7] + data.tbl[i][11];
                                	run_params.qNagb[index][j] = data.tbl[i][8] + data.tbl[i][12];
                                	run_params.qOagb[index][j] = data.tbl[i][9];
                                	run_params.Qagb[index][j] = run_params.qCagb[index][j] + run_params.qNagb[index][j] + run_params.qOagb[index][j];              
                                	index++;
                        	}
                	}
        	}
	}

	/* READ SN II YIELD */
	if (run_params.SNIIYields == 0)
        {
		double Z_std[METALGRID] = {0.0, 1e-4, 4e-4, 4e-3, 8e-3, 0.02, 0.05}; //metal grid Woosley & Weaver 1995
		strcpy(fname, "src/auxdata/yields/table4a.dat");
                cols = 17; //number of columns in the file
		
		struct table data;	
		data = read_table(fname, cols);
		rows = data.nr;
		double Z[rows];

		//count how many rows necessary for each column based on Z
		int count = 0; 
        	for (i=0; i<rows; i++)
        	{
                	Z[i] = data.tbl[i][0];
                	if (Z[i] == Z_std[0]) {
                        	count++;}
        	}

		run_params.countsn = count;

        	for (j=0; j<7; j++){
                	int index = 0;
                	for (i=0; i<rows; i++){
                	        if (Z[i] == Z_std[j])
                        	{
                                	run_params.msn[index] = data.tbl[i][1];
                                	run_params.qCsn[index][j] = data.tbl[i][6] + data.tbl[i][15];
                                	run_params.qOsn[index][j] = data.tbl[i][7];
                                	run_params.qMgsn[index][j] = data.tbl[i][9];
                                	run_params.qSisn[index][j] = data.tbl[i][10];
                                	run_params.qSsn[index][j] = data.tbl[i][11];
                                	run_params.qCasn[index][j] = data.tbl[i][12];
                                	run_params.qFesn[index][j] = data.tbl[i][13];
                                	run_params.Qsn[index][j] = run_params.qCsn[index][j] + run_params.qOsn[index][j] + run_params.qMgsn[index][j] + run_params.qSisn[index][j] + run_params.qSsn[index][j] + run_params.qCasn[index][j] + run_params.qFesn[index][j];
					index++;
                        	}
                	}
        	}
	}

	else if (run_params.SNIIYields == 1) //from Nomoto et al. 2006
	{
		double Z_std[METALGRID] = {0.0, 0.001, 0.004, 0.02};
		strcpy(fname, "src/auxdata/yields/Nomoto.dat");
		cols = 13; //number of columns in the file

		struct table data;
		data = read_table(fname, cols);
		rows = data.nr;
		double Z[rows];

		//count how many rows necessary for each column based on Z
		int count = 0;
		for (i=0; i<rows; i++)
		{
			Z[i] = data.tbl[i][0];
			if (Z[i] == Z_std[0]) {
				count++;}
		}	
		
		run_params.countsn = count;

		for (j=0; j<4; j++){
			int index = 0;
			for (i=0; i<rows; i++){
				if (Z[i] == Z_std[j])
				{
					run_params.msn[index] = data.tbl[i][1];
					run_params.qCsn[index][j] = data.tbl[i][2] + data.tbl[i][11];
					run_params.qOsn[index][j] = data.tbl[i][3];
					run_params.qMgsn[index][j] = data.tbl[i][5];
					run_params.qSisn[index][j] = data.tbl[i][6];
					run_params.qSsn[index][j] = data.tbl[i][7];
					run_params.qCasn[index][j] = data.tbl[i][8];
					run_params.qFesn[index][j] = data.tbl[i][9];
					run_params.Qsn[index][j] = run_params.qCsn[index][j] + run_params.qOsn[index][j] + run_params.qMgsn[index][j] + run_params.qSisn[index][j] + run_params.qSsn[index][j] + run_params.qCasn[index][j] + run_params.qFesn[index][j];
					index++;
				}
			}
		}
	}	


        /* SNIa YIELD */
	if (run_params.SNIaYields == 0)
	{
		run_params.qCrsnia = 0.0168; //yields from Iwamoto 1999
		run_params.qFesnia = 0.587;
		run_params.qNisnia = 0.0314;
	}

	else if (run_params.SNIaYields == 1) //yields from Seitenzahl et al. 2013
	{
		run_params.qCrsnia = 0.00857;
		run_params.qFesnia = 0.622;
		run_params.qNisnia = 0.069;
	}
	printf("ends reading yieldi \n");
}

struct table read_table(char *fname, int ncols)
{
        int i, j;
        struct table dt;
        FILE *file;

        file = fopen(fname,"r");
	fscanf(file, "%*[^\n]"); //skips the first line

        //start reading the file
        i=0;
        while (!feof(file)) //while it is not at the end of the file
    {
        for (j=0; j<ncols; j++)
        {
            fscanf(file, " %lf", &dt.tbl[i][j]); //notice the space char in front of %lf to handle any number of spaces as delimiter
        }

        fscanf(file,"%*[^\n]"); //ignore the rest of the line (if ncols is less than the file's column size), and go to the next line
        i++;
    }

        fclose(file);

        //store number of rows
        dt.nr = i-1; //i is at the line AFTER the last line, so it's must be subtracted by 1

    return dt;
}

