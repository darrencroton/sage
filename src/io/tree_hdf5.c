#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

#include "../core_allvars.h"
#include "../core_proto.h"
#include "tree_hdf5.h"

// Local Variables //
static hid_t hdf5_file;

// Local Structs //

struct METADATA_NAMES 
{
  char name_NTrees[MAX_STRING_LEN];
  char name_totNHalos[MAX_STRING_LEN];
  char name_TreeNHalos[MAX_STRING_LEN];
}; 

// Local Proto-Types //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType);
int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute);
int32_t read_dataset(char *dataset_name, int32_t datatype, void *buffer);

// External Functions //

void load_tree_table_hdf5(int filenr, int *ntrees, int **treenhalos, int **treefirsthalo)
{
    char buf[MAX_STRING_LEN];
    int32_t totNHalos;
    int32_t status;

    struct METADATA_NAMES metadata_names;

    snprintf(buf, MAX_STRING_LEN - 1, "%s/%s.%d%s", run_params.SimulationDir, run_params.TreeName, filenr, run_params.TreeExtension);
    hdf5_file = H5Fopen(buf, H5F_ACC_RDONLY, H5P_DEFAULT);

    if (hdf5_file < 0) {
        printf("can't open file `%s'\n", buf);
        ABORT(0);    
    }

    status = fill_metadata_names(&metadata_names, run_params.TreeType);
    if (status != EXIT_SUCCESS) {
        ABORT(0);
    }
 
    status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_NTrees, ntrees);
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Error while processing file %s\n", buf);
        fprintf(stderr, "Error code is %d\n", status);
        ABORT(0);
    }
    const int local_ntrees = *ntrees;
    
    status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_totNHalos, &totNHalos);
    if (status != EXIT_SUCCESS) { 
        fprintf(stderr, "Error while processing file %s\n", buf);
        fprintf(stderr, "Error code is %d\n", status);
        ABORT(0);
    }
#ifdef MPI
    if(ThisTask == 0)
#endif
        printf("There are %d trees and %d total halos\n", local_ntrees, totNHalos);

  
    *treenhalos = mymalloc(sizeof(int) * local_ntrees); 
    int *local_treenhalos = *treenhalos;
    
    status = read_attribute_int(hdf5_file, "/Header", metadata_names.name_TreeNHalos, local_treenhalos);
    if (status != EXIT_SUCCESS) {
        fprintf(stderr, "Error while processing file %s\n", buf);
        fprintf(stderr, "Error code is %d\n", status);
        ABORT(0);
    }

    *treefirsthalo = mymalloc(sizeof(int) * local_ntrees);
    int *local_treefirsthalo = *treefirsthalo;

#ifdef MPI
    if(ThisTask == 0)
#endif        
        for (int i = 0; i < 20; ++i) {
            printf("Tree %d: NHalos %d\n", i, local_treenhalos[i]);
        }

    if(local_ntrees) {
        local_treefirsthalo[0] = 0;
    }
    for(int i = 1; i < local_ntrees; i++) {
        local_treefirsthalo[i] = local_treefirsthalo[i - 1] + local_treenhalos[i - 1];
    }

}

#define READ_TREE_PROPERTY(sage_name, hdf5_name, type_int, data_type)   \
    {                                                                   \
        snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/%s", treenr, #hdf5_name); \
        status = read_dataset(dataset_name, type_int, buffer);          \
        if (status != EXIT_SUCCESS) {                                   \
            ABORT(0);                                                   \
        }                                                               \
        for (int halo_idx = 0; halo_idx < nhalos; ++halo_idx) { \
            local_halos[halo_idx].sage_name = ((data_type*)buffer)[halo_idx];  \
        }                                                               \
    }                                                                   \

#define READ_TREE_PROPERTY_MULTIPLEDIM(sage_name, hdf5_name, type_int, data_type) \
    {                                                                   \
        snprintf(dataset_name, MAX_STRING_LEN - 1, "tree_%03d/%s", treenr, #hdf5_name); \
        status = read_dataset(dataset_name, type_int, buffer_multipledim); \
        if (status != EXIT_SUCCESS) {                                   \
            ABORT(0);                                                   \
        }                                                               \
        for (int halo_idx = 0; halo_idx < nhalos; ++halo_idx) { \
            for (int dim = 0; dim < NDIM; ++dim) {                      \
                local_halos[halo_idx].sage_name[dim] = ((data_type*)buffer_multipledim)[halo_idx * NDIM + dim]; \
            }                                                           \
        }                                                               \
    }                                                                   \


void load_tree_hdf5(int32_t treenr, const int32_t nhalos, struct halo_data **halos)
{

    char dataset_name[MAX_STRING_LEN];
    int32_t status;

    double *buffer; // Buffer to hold the read HDF5 data.  
    // The largest data-type will be double. 

    double *buffer_multipledim; // However also need a buffer three times as large to hold data such as position/velocity.

    if (hdf5_file <= 0) {
        fprintf(stderr, "The HDF5 file should still be opened when reading the halos in the tree.\n");
        fprintf(stderr, "For tree %d we encountered error\n", treenr);
        H5Eprint(hdf5_file, stderr);
        ABORT(0);
    }


    *halos = mymalloc(sizeof(struct halo_data) * nhalos); 
    struct halo_data *local_halos = *halos;
    
    buffer = calloc(nhalos, sizeof(*(buffer)));
    if (buffer == NULL) {
        fprintf(stderr, "Could not allocate memory for the HDF5 buffer.\n");
        ABORT(0);
    }

    buffer_multipledim = calloc(nhalos * NDIM, sizeof(*(buffer_multipledim))); 
    if (buffer_multipledim == NULL) {
        fprintf(stderr, "Could not allocate memory for the HDF5 multiple dimension buffer.\n");
        ABORT(0);
    }

    // We now need to read in all the halo fields for this tree.
    // To do so, we read the field into a buffer and then properly slot the field into the Halo struct.

    /* Merger Tree Pointers */ 
    READ_TREE_PROPERTY(Descendant, Descendant, 0, int);
    READ_TREE_PROPERTY(FirstProgenitor, FirstProgenitor, 0, int);
    READ_TREE_PROPERTY(NextProgenitor, NextProgenitor, 0, int);
    READ_TREE_PROPERTY(FirstHaloInFOFgroup, FirstHaloInFOFgroup, 0, int);
    READ_TREE_PROPERTY(NextHaloInFOFgroup, NextHaloInFOFgroup, 0, int);

    /* Halo Properties */ 
    READ_TREE_PROPERTY(Len, Len, 0, int);
    READ_TREE_PROPERTY(M_Mean200, M_mean200, 1, float);
    READ_TREE_PROPERTY(Mvir, Mvir, 1, float);
    READ_TREE_PROPERTY(M_TopHat, M_TopHat, 1, float);
    READ_TREE_PROPERTY_MULTIPLEDIM(Pos, Pos, 1, float);
    READ_TREE_PROPERTY_MULTIPLEDIM(Vel, Vel, 1, float);
    READ_TREE_PROPERTY(VelDisp, VelDisp, 1, float);
    READ_TREE_PROPERTY(Vmax, Vmax, 1, float);
    READ_TREE_PROPERTY_MULTIPLEDIM(Spin, Spin, 1, float);
    READ_TREE_PROPERTY(MostBoundID, MostBoundID, 2, long long);

    /* File Position Info */
    READ_TREE_PROPERTY(SnapNum, SnapNum, 0, int);
    READ_TREE_PROPERTY(FileNr, Filenr, 0, int);
    READ_TREE_PROPERTY(SubhaloIndex, SubHaloIndex, 0, int);
    READ_TREE_PROPERTY(SubHalfMass, SubHalfMass, 0, int);

    free(buffer);
    free(buffer_multipledim);

#ifdef DEBUG_HDF5_READER 
    for (int32_t i = 0; i < 20; ++i) {
        printf("halo %d: Descendant %d FirstProg %d x %.4f y %.4f z %.4f\n", i, local_halos[i].Descendant, local_halos[i].FirstProgenitor, local_halos[i].Pos[0], local_halos[i].Pos[1], local_halos[i].Pos[2]); 
    }
    ABORT(0);
#endif 
 
}

#undef READ_TREE_PROPERTY
#undef READ_TREE_PROPERTY_MULTIPLEDIM

void close_hdf5_file(void)
{

    H5Fclose(hdf5_file);

}

// Local Functions //

int32_t fill_metadata_names(struct METADATA_NAMES *metadata_names, enum Valid_TreeTypes my_TreeType)
{

    switch (my_TreeType)
        {

        case genesis_lhalo_hdf5: 
  
            snprintf(metadata_names->name_NTrees, MAX_STRING_LEN - 1, "Ntrees"); // Total number of trees within the file.
            snprintf(metadata_names->name_totNHalos, MAX_STRING_LEN - 1, "totNHalos"); // Total number of halos within the file.
            snprintf(metadata_names->name_TreeNHalos, MAX_STRING_LEN - 1, "TreeNHalos"); // Number of halos per tree within the file.
            return EXIT_SUCCESS;

        case lhalo_binary: 
            fprintf(stderr, "If the file is binary then this function should never be called.  Something's gone wrong...");
            return EXIT_FAILURE;

        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);

        }

    return EXIT_FAILURE;

}

int32_t read_attribute_int(hid_t my_hdf5_file, char *groupname, char *attr_name, int *attribute)
{

    int32_t status;
    hid_t attr_id; 

    attr_id = H5Aopen_by_name(my_hdf5_file, groupname, attr_name, H5P_DEFAULT, H5P_DEFAULT);
    if (attr_id < 0) {
        fprintf(stderr, "Could not open the attribute %s in group %s\n", attr_name, groupname);
        return attr_id;
    }

    status = H5Aread(attr_id, H5T_NATIVE_INT, attribute);
    if (status < 0) {
        fprintf(stderr, "Could not read the attribute %s in group %s\n", attr_name, groupname);
        return status;
    }
    
    status = H5Aclose(attr_id);
    if (status < 0) {
        fprintf(stderr, "Error when closing the file.\n"); 
        H5Eprint(status, stderr);
        return status;
    }
   
    return EXIT_SUCCESS; 
}

int32_t read_dataset(char *dataset_name, int32_t datatype, void *buffer)
{
    hid_t dataset_id;

    dataset_id = H5Dopen2(hdf5_file, dataset_name, H5P_DEFAULT);
    if (dataset_id < 0) {
        fprintf(stderr, "Error encountered when trying to open up dataset %s\n", dataset_name); 
        H5Eprint(dataset_id, stderr);
        return dataset_id;
    }

    switch (datatype)
        {

        case 0:
            H5Dread(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);  
            break;

        case 1:
            H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
            break;
        case 2:
            H5Dread(dataset_id, H5T_NATIVE_LLONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);            
            break;
            
        default:
            fprintf(stderr, "Your data type has not been included in the switch statement for ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);

        }
  
    return EXIT_SUCCESS;
}
