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

// Local Variables //

FILE *load_fd;

// Local Proto-Types //

// External Functions //

#ifndef MAX_BUF_SIZE
#define MAX_BUF_SIZE (3*MAX_STRING_LEN+40)
#endif

void load_tree_table_binary(int32_t filenr)
{
  int i, totNHalos;
  char buf[MAX_BUF_SIZE+1];

        // open the file each time this function is called
  snprintf(buf, MAX_BUF_SIZE, "%s/%s.%d%s", SimulationDir, TreeName, filenr, TreeExtension);
  if(!(load_fd = fopen(buf, "r")))
  {
    printf("can't open file `%s'\n", buf);
    ABORT(0);
  }

  myfread(&Ntrees, 1, sizeof(int), load_fd);
  myfread(&totNHalos, 1, sizeof(int), load_fd);

  TreeNHalos = mymalloc(sizeof(int) * Ntrees);
  TreeFirstHalo = mymalloc(sizeof(int) * Ntrees);

  myfread(TreeNHalos, Ntrees, sizeof(int), load_fd);

  if(Ntrees)
    TreeFirstHalo[0] = 0;
  for(i = 1; i < Ntrees; i++)
    TreeFirstHalo[i] = TreeFirstHalo[i - 1] + TreeNHalos[i - 1];

}

void load_tree_binary(int32_t filenr, int32_t treenr)
{
  // must have an FD
  assert(load_fd );

  Halo = mymalloc(sizeof(struct halo_data) * TreeNHalos[treenr]);

  myfread(Halo, TreeNHalos[treenr], sizeof(struct halo_data), load_fd);

}

void close_binary_file(void)
{
  if(load_fd)
  {
    fclose(load_fd);
    load_fd = NULL;
  }
}
// Local Functions //
