#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_mymalloc.h"
#include "core_io_tree.h"

#include "io/read_tree_binary.h"
#ifdef HDF5
#include "io/read_tree_hdf5.h"
#include "io/read_tree_genesis_standard_hdf5.h"
#endif

void load_forest_table(const enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info)
{
    switch (my_TreeType)
        {
#ifdef HDF5
        case genesis_lhalo_hdf5:
            load_forest_table_hdf5(forests_info);
            break;
            
        case genesis_standard_hdf5:
            load_forest_table_genesis_hdf5(forests_info);
            break;
#endif

        case lhalo_binary:
            load_forest_table_binary(forests_info);
            break;

        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for function ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);
        }

}

void free_forest_table(enum Valid_TreeTypes my_TreeType, struct forest_info *forests_info)
{
    /* Don't forget to free the open file handle */
    switch (my_TreeType) {
#ifdef HDF5
    case genesis_lhalo_hdf5:
        close_hdf5_file(forests_info);
        break;
        
    case genesis_standard_hdf5:
        close_genesis_hdf5_file(forests_info);
        break;
            
#endif
            
    case lhalo_binary:
        close_binary_file(forests_info);
        break;
        
    default:
        fprintf(stderr, "Your tree type has not been included in the switch statement for function ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
        fprintf(stderr, "Please add it there.\n");
        ABORT(EXIT_FAILURE);
        
    }
}

void load_forest(const int forestnr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos, struct forest_info *forests_info)
{

#ifndef HDF5
    (void) forestnr; /* forestnr is currently only used for the hdf5 files */
#endif    

    switch (my_TreeType) {
        
#ifdef HDF5
    case genesis_lhalo_hdf5:
        load_forest_hdf5(forestnr, nhalos, halos, forests_info);
        break;
        
    case genesis_standard_hdf5:
        load_forest_genesis_hdf5(forestnr, nhalos, halos, forests_info);
        break;
#endif            
        
    case lhalo_binary:
        load_forest_binary(nhalos, halos, forests_info);
        break;
        
    default:
        fprintf(stderr, "Your tree type has not been included in the switch statement for ``%s`` in ``%s``.\n",
                __FUNCTION__, __FILE__);
        fprintf(stderr, "Please add it there.\n");
        ABORT(EXIT_FAILURE);
    }
}

