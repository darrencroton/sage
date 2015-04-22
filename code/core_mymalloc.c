#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "core_allvars.h"
#include "core_proto.h"

#define MAXBLOCKS 256

static unsigned long Nblocks = 0;
static void *Table[MAXBLOCKS];
static size_t SizeTable[MAXBLOCKS];
static size_t TotMem = 0, HighMarkMem = 0, OldPrintedHighMark = 0;



void *mymalloc(size_t n)
{
  if((n % 8) > 0)
    n = (n / 8 + 1) * 8;

  if(n == 0)
    n = 8;

  if(Nblocks >= MAXBLOCKS)
  {
    printf("No blocks left in mymalloc().\n");
    ABORT(0);
  }

  SizeTable[Nblocks] = n;
  TotMem += n;
  if(TotMem > HighMarkMem)
  {
    HighMarkMem = TotMem;
    if(HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0)
    {
      printf("new high mark = %g MB\n", HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  if(!(Table[Nblocks] = malloc(n)))
  {
    printf("Failed to allocate memory for %g MB\n",  n / (1024.0 * 1024.0) );
    ABORT(0);
  }

  Nblocks += 1;

  return Table[Nblocks - 1];
}



void myfree(void *p)
{
	assert(Nblocks > 0);

  if(p != Table[Nblocks - 1])
  {
    printf("Wrong call of myfree() - not the last allocated block!\n");
    ABORT(0);
  }

  free(p);

  Nblocks -= 1;

  TotMem -= SizeTable[Nblocks];
}



void print_allocated(void)
{
  printf("allocated = %g MB\n", TotMem / (1024.0 * 1024.0));
}

