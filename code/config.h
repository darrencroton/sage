#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "util_error.h"

/* HDF5 configuration */
#ifdef HDF5
#include <hdf5.h>
#define MODELNAME "SAGE"
#endif

/* Legacy ABORT macro - redirects to new FATAL_ERROR macro for backward
 * compatibility */
#define ABORT(sigterm) FATAL_ERROR("Program aborted with exit code %d", sigterm)

/* Global configuration structure - replaces individual globals */
extern struct SageConfig SageConfig;

#endif /* #ifndef CONFIG_H */
