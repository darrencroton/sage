#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "config.h"
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
    FATAL_ERROR("Memory allocation limit reached: No blocks left in mymalloc(). Increase MAXBLOCKS (%d).", MAXBLOCKS);
  }

  SizeTable[Nblocks] = n;
  TotMem += n;
  if(TotMem > HighMarkMem)
  {
    HighMarkMem = TotMem;
    if(HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0)
    {
      INFO_LOG("New memory usage high mark: %.2f MB", HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  if(!(Table[Nblocks] = malloc(n)))
  {
    FATAL_ERROR("Memory allocation failed: Unable to allocate %.2f MB", n / (1024.0 * 1024.0));
  }

  Nblocks += 1;

  return Table[Nblocks - 1];
}


void *myrealloc(void *p, size_t n)
{
  if((n % 8) > 0)
    n = (n / 8 + 1) * 8;

  if(n == 0)
    n = 8;

   if(p != Table[Nblocks - 1])
   {
      FATAL_ERROR("Memory management violation: Wrong call of myrealloc() - pointer %p is not the last allocated block (%p)", 
                  p, Table[Nblocks-1]);
  }
  
  void *newp = realloc(Table[Nblocks-1], n);
  if(newp == NULL) {
      FATAL_ERROR("Memory reallocation failed: Unable to reallocate %.2f MB (old size = %.2f MB)",
                  n / (1024.0 * 1024.0), SizeTable[Nblocks-1] / (1024.0 * 1024.0));
   }
  Table[Nblocks-1] = newp;
  
  TotMem -= SizeTable[Nblocks-1];
  TotMem += n;
  SizeTable[Nblocks-1] = n;
  
  if(TotMem > HighMarkMem)
  {
    HighMarkMem = TotMem;
    if(HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0)
    {
      INFO_LOG("New memory usage high mark: %.2f MB", HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  return Table[Nblocks - 1];
}


void myfree(void *p)
{
	assert(Nblocks > 0);

  if(p != Table[Nblocks - 1])
  {
    FATAL_ERROR("Memory management violation: Wrong call of myfree() - pointer %p is not the last allocated block (%p)",
                p, Table[Nblocks-1]);
  }

  free(p);

  Nblocks -= 1;

  TotMem -= SizeTable[Nblocks];
}



void print_allocated(void)
{
  INFO_LOG("Memory currently allocated: %.2f MB", TotMem / (1024.0 * 1024.0));
}

