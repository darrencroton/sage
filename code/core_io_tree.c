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
#include "io/tree_hdf5.h"

// keep a static file handle to remove the need to do constant seeking
FILE* load_fd = NULL;

#ifdef HDF5
hid_t hdf5_file; 
#endif

void load_tree_table(int filenr, enum Valid_TreeTypes my_TreeType)
{
  switch (my_TreeType)
  {

    case genesis_lhalo_hdf5:
      load_tree_table_hdf5(filenr, hdf5_file);
      break;

    case lhalo_binary:
      load_tree_table_binary(filenr, load_fd);
      break;

    default :
      fprintf(stderr, "The specified TreeType was my_TreeType\nThis is not a valid option.");
      ABORT(0);
  }

}

void free_tree_table(void)
{
  int n;

  for(n = NOUT - 1; n >= 0; n--)
    myfree(TreeNgals[n]);

  myfree(TreeFirstHalo);
  myfree(TreeNHalos);
	
	// Don't forget to free the open file handle
	if(load_fd) {
		fclose(load_fd);
		load_fd = NULL;
	}
}



void load_tree(int filenr, int treenr, enum Valid_TreeTypes my_TreeType)
{
  switch (my_TreeType)
  {

    case genesis_lhalo_hdf5:
      load_tree_hdf5(filenr, treenr, hdf5_file);
      break;

    case lhalo_binary:
      load_tree_binary(filenr, treenr, load_fd);
      break;

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
