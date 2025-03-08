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

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3*MAX_STRING_LEN+40)
#endif

void load_tree_table(int filenr, enum Valid_TreeTypes my_TreeType)
{
  int i, n;
  FILE *fd;
  char buf[MAX_BUF_SIZE+1];

  switch (my_TreeType)
  {
#ifdef HDF5
    case genesis_lhalo_hdf5:
      load_tree_table_hdf5(filenr);
      break;
#endif

    case lhalo_binary:
      load_tree_table_binary(filenr);
      break;

    default:
      fprintf(stderr, "Unsupported tree type %d in load_tree_table(). Please add support in core_io_tree.c\n", my_TreeType);
      ABORT(EXIT_FAILURE);
  }

  for(n = 0; n < NOUT; n++)
  {
    TreeNgals[n] = mymalloc(sizeof(int) * Ntrees);
    if(TreeNgals[n] == NULL)
    {
      fprintf(stderr, "Error: Memory allocation failed for TreeNgals[%d] array (%d trees, %zu bytes)\n", 
              n, Ntrees, Ntrees * sizeof(int));
      ABORT(0);
    }
    SimState.TreeNgals[n] = TreeNgals[n]; /* Update SimState pointer directly */
    
    for(i = 0; i < Ntrees; i++)
      TreeNgals[n][i] = 0;

    snprintf(buf, MAX_BUF_SIZE, "%s/%s_z%1.3f_%d", SageConfig.OutputDir, SageConfig.FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

    if(!(fd = fopen(buf, "w")))
    {
      fprintf(stderr, "Error: Failed to create output galaxy file '%s' for snapshot %d (filenr %d)\n", 
              buf, ListOutputSnaps[n], filenr);
      ABORT(0);
    }
    fclose(fd);
    TotGalaxies[n] = 0;
    SimState.TotGalaxies[n] = 0; /* Update SimState directly */
  }

}

void free_tree_table(enum Valid_TreeTypes my_TreeType)
{
  int n;

  for(n = NOUT - 1; n >= 0; n--)
  {
    myfree(TreeNgals[n]);
    TreeNgals[n] = NULL;
    SimState.TreeNgals[n] = NULL; /* Update SimState pointer */
  }

  myfree(TreeFirstHalo);
  TreeFirstHalo = NULL;
  SimState.TreeFirstHalo = NULL; /* Update SimState pointer */

  myfree(TreeNHalos);
  TreeNHalos = NULL;
  SimState.TreeNHalos = NULL; /* Update SimState pointer */

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
      fprintf(stderr, "Unsupported tree type %d in free_tree_table(). Please add support in core_io_tree.c\n", my_TreeType);
      ABORT(EXIT_FAILURE);

  }

}

void load_tree(int filenr, int treenr, enum Valid_TreeTypes my_TreeType)
{
  int32_t i;

  switch (my_TreeType)
  {

#ifdef HDF5
  case genesis_lhalo_hdf5:
      load_tree_hdf5(filenr, treenr);
      break;
#endif
    case lhalo_binary:
      load_tree_binary(filenr, treenr);
      break;

    default:
      fprintf(stderr, "Unsupported tree type %d in load_tree(). Please add support in core_io_tree.c\n", my_TreeType);
      ABORT(EXIT_FAILURE);

  }

  MaxGals = (int)(MAXGALFAC * TreeNHalos[treenr]);
  if(MaxGals < 10000)
    MaxGals = 10000;

  FoF_MaxGals = 10000;
  
  /* Update SimulationState */
  SimState.MaxGals = MaxGals;
  SimState.FoF_MaxGals = FoF_MaxGals;
  sync_sim_state_to_globals();

  HaloAux = mymalloc(sizeof(struct halo_aux_data) * TreeNHalos[treenr]);
  if(HaloAux == NULL)
  {
    fprintf(stderr, "Error: Memory allocation failed for HaloAux array (%d halos, %zu bytes)\n", 
            TreeNHalos[treenr], TreeNHalos[treenr] * sizeof(struct halo_aux_data));
    ABORT(0);
  }
  
  HaloGal = mymalloc(sizeof(struct GALAXY) * MaxGals);
  if(HaloGal == NULL)
  {
    fprintf(stderr, "Error: Memory allocation failed for HaloGal array (%d galaxies, %zu bytes)\n", 
            MaxGals, MaxGals * sizeof(struct GALAXY));
    ABORT(0);
  }
  
  Gal = mymalloc(sizeof(struct GALAXY) * FoF_MaxGals);
  if(Gal == NULL)
  {
    fprintf(stderr, "Error: Memory allocation failed for Gal array (%d galaxies, %zu bytes)\n", 
            FoF_MaxGals, FoF_MaxGals * sizeof(struct GALAXY));
    ABORT(0);
  }

  for(i = 0; i < TreeNHalos[treenr]; i++)
  {
    HaloAux[i].DoneFlag = 0;
    HaloAux[i].HaloFlag = 0;
    HaloAux[i].NGalaxies = 0;
  }


}

void free_galaxies_and_tree(void)
{
  myfree(Gal);
  myfree(HaloGal);
  myfree(HaloAux);
  myfree(Halo);
}

size_t myfread(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  return fread(ptr, size, nmemb, stream);
}

size_t myfwrite(void *ptr, size_t size, size_t nmemb, FILE * stream)
{
  return fwrite(ptr, size, nmemb, stream);
}

int myfseek(FILE * stream, long offset, int whence)
{
  return fseek(stream, offset, whence);
}
