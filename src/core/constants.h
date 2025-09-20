#ifndef CONSTANTS_H
#define CONSTANTS_H

/* Floating-point comparison epsilon values */
#define EPSILON_SMALL 1.0e-10 /* For near-zero comparisons */
#define EPSILON_MEDIUM 1.0e-6 /* For general equality comparisons */
#define EPSILON_LARGE 1.0e-4  /* For physics model thresholds */

/* Numerical constants for the simulation */
#define NDIM 3
#define STEPS 10 /* Number of integration intervals between two snapshots */
#define MAXGALFAC 1
#define ALLOCPARAMETER 10.0
#define MAX_NODE_NAME_LEN 50
#define ABSOLUTEMAXSNAPS                                                       \
  1000              /* The largest number of snapshots for any simulation */
#define MAXTAGS 300 /* Max number of parameters */
#define MAX_STRING_LEN 1024 /* Max length of a string containing a name */

/* Memory allocation parameters */
#define GALAXY_ARRAY_GROWTH_FACTOR                                             \
  1.5 /* Factor to grow arrays by (1.5 = 50% growth) */
#define MIN_GALAXY_ARRAY_GROWTH                                                \
  1000 /* Minimum growth increment regardless of factor */
#define MAX_GALAXY_ARRAY_SIZE                                                  \
  1000000000 /* Upper limit to prevent excessive allocation */
#define INITIAL_FOF_GALAXIES 1000 /* Initial size for FOF galaxy arrays */

/* Physical constants */
#define GRAVITY 6.672e-8
#define SOLAR_MASS 1.989e33
#define SOLAR_LUM 3.826e33
#define RAD_CONST 7.565e-15
#define AVOGADRO 6.0222e23
#define BOLTZMANN 1.3806e-16
#define GAS_CONST 8.31425e7
#define C 2.9979e10
#define PLANCK 6.6262e-27
#define CM_PER_MPC 3.085678e24
#define PROTONMASS 1.6726e-24
#define HUBBLE 3.2407789e-18 /* in h/sec */

#define SEC_PER_MEGAYEAR 3.155e13
#define SEC_PER_YEAR 3.155e7

/* Data type IDs */
#define DOUBLE 1
#define STRING 2
#define INT 3

#endif /* #ifndef CONSTANTS_H */
