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
#include "tree_binary.h"

// Local Proto-Types //

// External Functions //

void load_tree_table_binary(int32_t filenr, FILE *my_load_fd)
{
  int i, n, totNHalos;
  char buf[MAX_STRING_LEN];
  FILE *fd;

	// open the file each time this function is called
  sprintf(buf, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
  if(!(my_load_fd = fopen(buf, "r")))
  {
    printf("can't open file `%s'\n", buf);
    ABORT(0);
  }

  myfread(&Ntrees, 1, sizeof(int), my_load_fd);
  myfread(&totNHalos, 1, sizeof(int), my_load_fd);

  TreeNHalos = mymalloc(sizeof(int) * Ntrees);
  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);

  for(n = 0; n < NOUT; n++)
    TreeNgals[n] = mymalloc(sizeof(int) * Ntrees);
  myfread(TreeNHalos, Ntrees, sizeof(int), my_load_fd);

  if(Ntrees)
    TreeFirstHalo[0] = 0;
  for(i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];

  for(n = 0; n < NOUT; n++)
  {
    for(i = 0; i < Ntrees; i++)
      TreeNgals[n][i] = 0;

    sprintf(buf, "%s/%s_z%1.3f_%d", OutputDir, FileNameGalaxies, ZZ[ListOutputSnaps[n]], filenr);

    if(!(fd = fopen(buf, "w")))
    {
      printf("can't open file `%s'\n", buf);
      ABORT(0);
    }
    fclose(fd);
    TotGalaxies[n] = 0;
  }
}

void load_tree_binary(int32_t filenr, int32_t treenr, FILE *my_load_fd)
{
  int32_t i;

  // must have an FD
  assert( my_load_fd );

  Halo = mymalloc(sizeof(struct halo_data) * TreeNHalos[treenr]);

  myfread(Halo, TreeNHalos[treenr], sizeof(struct halo_data), my_load_fd);

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

// Local Functions //

