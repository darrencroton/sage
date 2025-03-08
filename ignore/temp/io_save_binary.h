#ifndef IO_SAVE_BINARY_H
#define IO_SAVE_BINARY_H

#include "types.h"
#include "globals.h"
#include "config.h"

extern void save_galaxies(int filenr, int tree);
extern void prepare_galaxy_for_output(int filenr, int tree, struct GALAXY *g, struct GALAXY_OUTPUT *o);
extern void finalize_galaxy_file(int filenr);

#endif /* #ifndef IO_SAVE_BINARY_H */
