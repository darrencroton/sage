/* File: macros.h */
/*
  This file is a part of the Corrfunc package
  Copyright (C) 2015-- Manodeep Sinha (manodeep@gmail.com)
  License: MIT LICENSE. See LICENSE file under the top-level
  directory at https://github.com/manodeep/Corrfunc/
*/


#pragma once


#ifdef HDF5
#include <hdf5.h>
#define MODELNAME        "SAGE"
#endif

#define NDIM 3

#define ABORT(sigterm)                                              \
    do {                                                                \
        printf("Error in file: %s\tfunc: %s\tline: %i\n", __FILE__, __FUNCTION__, __LINE__); \
        printf("exit code = %d\n", sigterm);                            \
    } while(0)

#define  STEPS 10         /* Number of integration intervals between two snapshots */
#define  MAXGALFAC 1
#define  ALLOCPARAMETER 10.0
#define  MAX_NODE_NAME_LEN 50
#define  ABSOLUTEMAXSNAPS 1000  /* The largest number of snapshots for any simulation */
#define  MAXTAGS          300  /* Max number of parameters */


#define  GRAVITY     6.672e-8
#define  SOLAR_MASS  1.989e33
#define  SOLAR_LUM   3.826e33
#define  RAD_CONST   7.565e-15
#define  AVOGADRO    6.0222e23
#define  BOLTZMANN   1.3806e-16
#define  GAS_CONST   8.31425e7
#define  C           2.9979e10
#define  PLANCK      6.6262e-27
#define  CM_PER_MPC  3.085678e24
#define  PROTONMASS  1.6726e-24
#define  HUBBLE      3.2407789e-18   /* in h/sec */

#define  SEC_PER_MEGAYEAR   3.155e13
#define  SEC_PER_YEAR       3.155e7

#define  MAX_STRING_LEN     1024 /* Max length of a string containing a name */

#define ADD_DIFF_TIME(t0, t1) ((t1.tv_sec - t0.tv_sec) + 1e-6 * (t1.tv_usec - t0.tv_usec))
#define REALTIME_ELAPSED_NS(t0, t1)                                                                \
  ((t1.tv_sec - t0.tv_sec) * 1000000000.0 + (t1.tv_nsec - t0.tv_nsec))

#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)

/* Taken from
   http://stackoverflow.com/questions/19403233/compile-time-struct-size-check-error-out-if-odd which
   is in turn taken from the linux kernel */
/* #define BUILD_BUG_OR_ZERO(e) (sizeof(struct{ int:-!!(e);})) */
/* #define ENSURE_STRUCT_SIZE(e, size)  BUILD_BUG_OR_ZERO(sizeof(e) != size) */
/* However, the previous one gives me an unused-value warning and I do not want
   to turn that compiler warning off. Therefore, this version, which results in
   an unused local typedef warning is used. I turn off the corresponding warning
   in common.mk (-Wno-unused-local-typedefs) via CFLAGS
*/
#define BUILD_BUG_OR_ZERO(cond, msg) typedef volatile char assertion_on_##msg[(!!(cond)) * 2 - 1]
#define ENSURE_STRUCT_SIZE(e, size)                                                                \
  BUILD_BUG_OR_ZERO(sizeof(e) == size, sizeof_struct_config_options)

/* Macro Constants */
// Just to output some colors
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET "\x1b[0m"

/* Function-like macros */
#ifdef NDEBUG
#define XASSERT(EXP, ...)                                                                          \
  do {                                                                                             \
  } while (0)
#else
#define XASSERT(EXP, ...)                                                                          \
  do {                                                                                             \
    if (!(EXP)) {                                                                                  \
      fprintf(stderr, "Error in file: %s\tfunc: %s\tline: %d with expression `" #EXP "'\n",        \
              __FILE__, __FUNCTION__, __LINE__);                                                   \
      fprintf(stderr, __VA_ARGS__);                                                                \
      fprintf(stderr, ANSI_COLOR_BLUE "Hopefully, input validation. Otherwise, bug in code: "      \
                                      "please email Manodeep Sinha "                               \
                                      "<manodeep@gmail.com>" ANSI_COLOR_RESET "\n");               \
      return EXIT_FAILURE;                                                                         \
    }                                                                                              \
  } while (0)
#endif

#ifdef NDEBUG
#define XPRINT(EXP, ...)                                                                           \
  do {                                                                                             \
  } while (0)
#else
#define XPRINT(EXP, ...)                                                                           \
  do {                                                                                             \
    if (!(EXP)) {                                                                                  \
      fprintf(stderr, "Error in file: %s\tfunc: %s\tline: %d with expression `" #EXP "'\n",        \
              __FILE__, __FUNCTION__, __LINE__);                                                   \
      fprintf(stderr, __VA_ARGS__);                                                                \
      fprintf(stderr, ANSI_COLOR_BLUE "Hopefully, input validation. Otherwise, bug in code: "      \
                                      "please email Manodeep Sinha "                               \
                                      "<manodeep@gmail.com>" ANSI_COLOR_RESET "\n");               \
    }                                                                                              \
  } while (0)
#endif

#ifdef NDEBUG
#define XRETURN(EXP, VAL, ...)                                                                     \
  do {                                                                                             \
  } while (0)
#else
#define XRETURN(EXP, VAL, ...)                                                                     \
  do {                                                                                             \
    if (!(EXP)) {                                                                                  \
      fprintf(stderr, "Error in file: %s\tfunc: %s\tline: %d with expression `" #EXP "'\n",        \
              __FILE__, __FUNCTION__, __LINE__);                                                   \
      fprintf(stderr, __VA_ARGS__);                                                                \
      fprintf(stderr, ANSI_COLOR_BLUE "Hopefully, input validation. Otherwise, bug in code: "      \
                                      "please email Manodeep Sinha "                               \
                                      "<manodeep@gmail.com>" ANSI_COLOR_RESET "\n");               \
      return VAL;                                                                                  \
    }                                                                                              \
  } while (0)
#endif

