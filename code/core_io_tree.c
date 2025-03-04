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
      fprintf(stderr, "Your tree type has not been included in the switch statement for ``load_tree_table`` in ``core_io_tree.c``.\n");
      fprintf(stderr, "Please add it there.\n");
      ABORT(EXIT_FAILURE);
  }

  for(n = 0; n < NOUT; n++)
  {
    TreeNgals[n] = mymalloc(sizeof(int) * Ntrees);
    for(i = 0; i < Ntrees; i++)
      TreeNgals[n][i] = 0;

    snprintf(buf, MAX_BUF_SIZE, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

    if(!(fd = fopen(buf, "w")))
    {
      printf("can't open file `%s'\n", buf);
      ABORT(0);
    }
    fclose(fd);
    TotGalaxies[n] = 0;
  }

}

void free_tree_table(enum Valid_TreeTypes my_TreeType)
{
  int n;

  for(n = NOUT - 1; n >= 0; n--)
    myfree(TreeNgals[n]);

  myfree(TreeFirstHalo);
  myfree(TreeNHalos);

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
      fprintf(stderr, "Your tree type has not been included in the switch statement for ``load_tree_table`` in ``core_io_tree.c``.\n");
      fprintf(stderr, "Please add it there.\n");
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
      fprintf(stderr, "Your tree type has not been included in the switch statement for ``load_tree`` in ``core_io_tree.c``.\n");
      fprintf(stderr, "Please add it there.\n");
      ABORT(EXIT_FAILURE);

  }

  MaxGals = (int)(MAXGALFAC * TreeNHalos[treenr]);
  if(MaxGals < 10000)
    MaxGals = 10000;

  FoF_MaxGals = 10000;

  HaloAux = mymalloc(sizeof(struct halo_aux_data) * TreeNHalos[treenr]);
  HaloGal = mymalloc(sizeof(struct GALAXY) * MaxGals);
  Gal = mymalloc(sizeof(struct GALAXY) * FoF_MaxGals);

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
