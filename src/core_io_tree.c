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
#include "core_proto.h"

#include "io/tree_binary.h"
#ifdef HDF5
#include "io/tree_hdf5.h"
#endif

#ifdef OLD_VERSION
void load_tree_table(int filenr, enum Valid_TreeTypes my_TreeType)
#else
void load_tree_table(const int filenr, const enum Valid_TreeTypes my_TreeType, int *ntrees, int **treenhalos, int **treefirsthalo, int **treengals, int *totgalaxies)
#endif
{

#ifdef OLD_VERSION
    int *ntrees = &Ntrees;
    int **treenhalos = &TreeNHalos;
    int **treefirsthalo = &TreeFirstHalo;
    int **treengals = (int **) TreeNgals;
    int *totgalaxies = (int *) TotGalaxies;
#endif 
    
    switch (my_TreeType)
        {
#ifdef HDF5
        case genesis_lhalo_hdf5:
            load_tree_table_hdf5(filenr, ntrees, treenhalos, treefirsthalo);
            break;
#endif

        case lhalo_binary:
            load_tree_table_binary(filenr, ntrees, treenhalos, treefirsthalo);
            break;

        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for function ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);
        }

    const int local_ntrees = *ntrees;
    for(int n = 0; n < NOUT; n++) {
        treengals[n] = mymalloc(sizeof(int) * local_ntrees);
        for(int i = 0; i < local_ntrees; i++) {
            treengals[n][i] = 0;
        }
        char buf[MAX_STRING_LEN + 1];
        snprintf(buf, MAX_STRING_LEN, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

        FILE *fd = fopen(buf, "w"); 
        if(fd == NULL) {
            printf("can't open file `%s'\n", buf);
            ABORT(0);
        }
        fclose(fd);
        totgalaxies[n] = 0;
    }
}

#ifdef OLD_VERSION
void free_tree_table(enum Valid_TreeTypes my_TreeType)
#else
void free_tree_table(enum Valid_TreeTypes my_TreeType, int **treengals, int *treenhalos, int *treefirsthalo)    
#endif    
{
    
#ifdef OLD_VERSION
    int **treengals = (int **) TreeNgals;
    int *treenhalos = TreeNHalos;
    int *treefirsthalo = TreeFirstHalo;
#endif    


    for(int n = 0; n < NOUT; n++) {
        myfree(treengals[n]);
    }

    myfree(treenhalos);
    myfree(treefirsthalo);

    // Don't forget to free the open file handle
    switch (my_TreeType)
        {
#ifdef HDF5
        case genesis_lhalo_hdf5:
            close_hdf5_file();
            break;
#endif
            
        case lhalo_binary:
            close_binary_file();
            break;
            
        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for function ``%s`` in file ``%s``.\n", __FUNCTION__, __FILE__);
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);
            
        }
}


#ifdef OLD_VERSION
int load_tree(const int treenr, const int nhalos, enum Valid_TreeTypes my_TreeType)
#else
int load_tree(const int treenr, const int nhalos, enum Valid_TreeTypes my_TreeType, struct halo_data **halos,
               struct halo_aux_data **haloaux, struct GALAXY **galaxies, struct GALAXY **halogal)
#endif    
{
#ifdef OLD_VERSION
    struct halo_data **halos = &Halo;
    struct halo_aux_data **haloaux = &HaloAux;
    struct GALAXY **galaxies = &Gal;
    struct GALAXY **halogal = &HaloGal;
#endif    

#ifndef HDF5
    (void) treenr; /* treenr is only used for the hdf5 files */
#endif    

    
    switch (my_TreeType)
        {
            
#ifdef HDF5
        case genesis_lhalo_hdf5:
            load_tree_hdf5(treenr, nhalos, halos);
            break;
#endif            
            
        case lhalo_binary:
            load_tree_binary(nhalos, halos);
            break;
            
        default:
            fprintf(stderr, "Your tree type has not been included in the switch statement for ``load_tree`` in ``core_io_tree.c``.\n");
            fprintf(stderr, "Please add it there.\n");
            ABORT(EXIT_FAILURE);
            
        }

    int maxgals = (int)(MAXGALFAC * nhalos);
    if(maxgals < 10000) maxgals = 10000;

    *haloaux = mymalloc(sizeof(struct halo_aux_data) * nhalos);
    *halogal = mymalloc(sizeof(struct GALAXY) * maxgals);
    *galaxies = mymalloc(sizeof(struct GALAXY) * maxgals);/* used to be fof_maxgals instead of maxgals*/

    struct halo_aux_data *tmp_halo_aux = *haloaux;
    for(int i = 0; i < nhalos; i++) {
        tmp_halo_aux->DoneFlag = 0;
        tmp_halo_aux->HaloFlag = 0;
        tmp_halo_aux->NGalaxies = 0;
        tmp_halo_aux++;
    }

    return maxgals;
}

#ifdef OLD_VERSION
void free_galaxies_and_tree(void)
#else
void free_galaxies_and_tree(struct GALAXY *galaxies, struct GALAXY *halogal, struct halo_aux_data *haloaux, struct halo_data *halos)
#endif    
{
#ifdef OLD_VERSION
    struct GALAXY *galaxies = Gal;
    struct GALAXY *halogal = HaloGal;
    struct halo_aux_data *haloaux = HaloAux;
    struct halo_data *halos = Halo; 
#endif

    myfree(galaxies);
    myfree(halogal);
    myfree(haloaux);
    myfree(halos);
}

size_t myfread(void *ptr, const size_t size, const size_t nmemb, FILE * stream)
{
    return fread(ptr, size, nmemb, stream);
}

size_t myfwrite(const void *ptr, const size_t size, const size_t nmemb, FILE * stream)
{
    return fwrite(ptr, size, nmemb, stream);
}

int myfseek(FILE * stream, const long offset, const int whence)
{
    return fseek(stream, offset, whence);
}
