#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

/* HDF5 configuration */
#ifdef HDF5
#include <hdf5.h>
#define MODELNAME "SAGE"
#endif

/* Abort macro for error handling */
#define ABORT(sigterm)                                                  \
do {                                                                \
  printf("Error in file: %s\tfunc: %s\tline: %i\n", __FILE__, __FUNCTION__, __LINE__); \
  myexit(sigterm);                                                \
} while(0)

/* Recipe flags */
extern int ReionizationOn;
extern int SupernovaRecipeOn;
extern int DiskInstabilityOn;
extern int AGNrecipeOn;
extern int SFprescription;

/* Tree type configuration */
extern enum Valid_TreeTypes TreeType;

#endif /* #ifndef CONFIG_H */
