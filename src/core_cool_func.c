#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "core_allvars.h"
#include "core_proto.h"


#define TABSIZE 91
#define LAST_TAB_INDEX (TABSIZE - 1)


static char *name[] = {
	"stripped_mzero.cie",
	"stripped_m-30.cie",
	"stripped_m-20.cie",
	"stripped_m-15.cie",
	"stripped_m-10.cie",
	"stripped_m-05.cie",
	"stripped_m-00.cie",
	"stripped_m+05.cie"
};


// Metallicies with repect to solar. Will be converted to absolut metallicities by adding log10(Z_sun), Zsun=0.02 
static double metallicities[8] = {
	-5.0,   // actually primordial -> -infinity 
	-3.0,
	-2.0,
	-1.5,
	-1.0,
	-0.5,
	+0.0,
	+0.5
};

double get_rate(int tab, double logTemp);

#define NUM_METALS_TABLE        sizeof(metallicities)/sizeof(metallicities[0])

static double CoolRate[NUM_METALS_TABLE][TABSIZE];

void read_cooling_functions(void)
{
  char buf[MAX_STRING_LEN];
  
  const double log10_zerop02 = log10(0.02);
  for(size_t i = 0; i < NUM_METALS_TABLE; i++) {
    metallicities[i] += log10_zerop02;     // add solar metallicity 
  }

  for(size_t i = 0; i < NUM_METALS_TABLE; i++)
  {
    /* Concatenates the actual path to the root directory 
       The variable ROOT_DIR is defined in the Makefile. C token pasting 
       automatically concats the ROOT_DIR string and the "extra/..." string
     */
    snprintf(buf, MAX_STRING_LEN, ROOT_DIR "/src/auxdata/CoolFunctions/%s", name[i]);
    FILE *fd = fopen(buf, "r");
    if(fd == NULL) {
      printf("file `%s' not found\n", buf);
      ABORT(0);
    }
    for(int n = 0; n < TABSIZE; n++)
    {
      float sd_logLnorm;
      const int nitems = fscanf(fd, " %*f %*f %*f %*f %*f %f%*[^\n]",
				&sd_logLnorm);
      if(nitems != 1) {
	fprintf(stderr,"Error: Could not read cooling rate on line %d\n", n);
	ABORT(0);
      }

      CoolRate[i][n] = sd_logLnorm;
    }

    fclose(fd);
  }

#ifdef MPI
  if(ThisTask == 0)
#endif
    printf("cooling functions read\n\n");

}

double get_rate(int tab, double logTemp)
{
  int index;
  double rate1, rate2, rate, logTindex;
  const double dlogT = 0.05;
  const double inv_dlogT = 1.0/dlogT;

  if(logTemp < 4.0)
    logTemp = 4.0;

  index = (logTemp - 4.0) * inv_dlogT;
  if(index >= LAST_TAB_INDEX) {
    /*MS: because index+1 is also accessed, therefore index can be at most LAST_TAB_INDEX */
    index = LAST_TAB_INDEX - 1;
  }

  logTindex = 4.0 + 0.05 * index;

  rate1 = CoolRate[tab][index];
  rate2 = CoolRate[tab][index + 1];

  rate = rate1 + (rate2 - rate1) * inv_dlogT * (logTemp - logTindex);

  return rate;
}


double get_metaldependent_cooling_rate(double logTemp, double logZ)  // pass: log10(temperatue/Kelvin), log10(metallicity) 
{
  int i;
  double rate1, rate2, rate;

  if(logZ < metallicities[0])
    logZ = metallicities[0];

  if(logZ > metallicities[7])
    logZ = metallicities[7];

  i = 0;
  while(logZ > metallicities[i + 1])
    i++;

  // look up at i and i+1 
  rate1 = get_rate(i, logTemp);
  rate2 = get_rate(i + 1, logTemp);

  rate = rate1 + (rate2 - rate1) / (metallicities[i + 1] - metallicities[i]) * (logZ - metallicities[i]);

  return pow(10.0, rate);
}



