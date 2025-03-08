#ifndef IO_SAVE_HDF5_H
#define IO_SAVE_HDF5_H

#include "../types.h"
#include "../globals.h"
#include "../config.h"

extern void calc_hdf5_props(void);
extern void write_hdf5_galaxy(struct GALAXY_OUTPUT *galaxy, int n, int filenr);
extern void write_hdf5_attrs(int n, int filenr);
extern void free_hdf5_ids(void);
extern void write_master_file(void);
extern void prep_hdf5_file(char* fname);

#endif /* #ifndef IO_SAVE_HDF5_H */
extern void free_hdf5_ids(void);
extern void write_master_file(void);
extern void prep_hdf5_file(char* fname);
