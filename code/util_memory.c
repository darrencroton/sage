/**
 * @file    core_mymalloc.c
 * @brief   Custom memory management system with tracking and LIFO constraints
 *
 * This file implements a custom memory allocation and tracking system that:
 * - Tracks memory usage and reports high watermarks
 * - Enforces Last-In-First-Out (LIFO) allocation/deallocation order
 * - Aligns memory allocations to 8-byte boundaries for performance
 * - Provides detailed error messages for memory-related issues
 *
 * The LIFO constraint helps catch memory management bugs but also imposes
 * limitations on how memory can be used in the code. This enforces a
 * structured approach to memory management throughout the codebase.
 *
 * Key functions:
 * - mymalloc(): Allocates memory with tracking
 * - myrealloc(): Reallocates memory (only for the most recently allocated block)
 * - myfree(): Frees memory (only for the most recently allocated block)
 * - print_allocated(): Reports current memory usage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "config.h"
#include "core_proto.h"
#include "util_memory.h"

/* Maximum number of memory blocks that can be tracked */
#define MAXBLOCKS 256

/* Memory tracking variables */
static unsigned long Nblocks = 0;            /* Number of allocated blocks */
static void *Table[MAXBLOCKS];               /* Pointers to allocated blocks */
static size_t SizeTable[MAXBLOCKS];          /* Sizes of allocated blocks */
static size_t TotMem = 0;                    /* Total allocated memory */
static size_t HighMarkMem = 0;               /* High watermark of memory usage */
static size_t OldPrintedHighMark = 0;        /* Last reported high watermark */



/**
 * @brief   Allocates memory with tracking and alignment
 *
 * @param   n       Number of bytes to allocate
 * @return  Pointer to the allocated memory block
 *
 * This function allocates memory while:
 * 1. Aligning to 8-byte boundaries for performance
 * 2. Tracking allocation in the Table and SizeTable arrays
 * 3. Recording total memory usage and high watermarks
 * 4. Enforcing the maximum block limit (MAXBLOCKS)
 * 
 * Memory allocated with mymalloc() must be freed with myfree()
 * in reverse order of allocation (LIFO constraint).
 */
void *mymalloc(size_t n)
{
  /* Align memory to 8-byte boundaries for performance */
  if((n % 8) > 0)
    n = (n / 8 + 1) * 8;

  /* Ensure minimum allocation size */
  if(n == 0)
    n = 8;

  /* Check if we've reached the maximum number of tracked blocks */
  if(Nblocks >= MAXBLOCKS)
  {
    FATAL_ERROR("Memory allocation limit reached: No blocks left in mymalloc(). Increase MAXBLOCKS (%d).", MAXBLOCKS);
  }

  /* Record allocation size and update total */
  SizeTable[Nblocks] = n;
  TotMem += n;
  
  /* Update and potentially report high watermark */
  if(TotMem > HighMarkMem)
  {
    HighMarkMem = TotMem;
    /* Only report when we exceed the previous mark by at least 10MB */
    if(HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0)
    {
      INFO_LOG("New memory usage high mark: %.2f MB", HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  /* Attempt actual allocation */
  if(!(Table[Nblocks] = malloc(n)))
  {
    FATAL_ERROR("Memory allocation failed: Unable to allocate %.2f MB", n / (1024.0 * 1024.0));
  }

  /* Increment block count and return pointer */
  Nblocks += 1;
  return Table[Nblocks - 1];
}


/**
 * @brief   Reallocates memory for the most recently allocated block
 *
 * @param   p       Pointer to memory block to be reallocated (must be last allocated)
 * @param   n       New size in bytes
 * @return  Pointer to the reallocated memory block
 *
 * This function reallocates memory while:
 * 1. Enforcing LIFO constraint (can only reallocate the most recently allocated block)
 * 2. Maintaining 8-byte alignment
 * 3. Updating tracking information
 * 4. Recording updated memory usage and high watermarks
 *
 * The LIFO constraint is crucial - attempting to reallocate any block
 * other than the most recently allocated will result in a fatal error.
 */
void *myrealloc(void *p, size_t n)
{
  /* Align memory to 8-byte boundaries */
  if((n % 8) > 0)
    n = (n / 8 + 1) * 8;

  /* Ensure minimum allocation size */
  if(n == 0)
    n = 8;

  /* Enforce LIFO constraint - can only reallocate the most recently allocated block */
  if(p != Table[Nblocks - 1])
  {
    FATAL_ERROR("Memory management violation: Wrong call of myrealloc() - pointer %p is not the last allocated block (%p)", 
                p, Table[Nblocks-1]);
  }
  
  /* Attempt reallocation */
  void *newp = realloc(Table[Nblocks-1], n);
  if(newp == NULL) {
    FATAL_ERROR("Memory reallocation failed: Unable to reallocate %.2f MB (old size = %.2f MB)",
                n / (1024.0 * 1024.0), SizeTable[Nblocks-1] / (1024.0 * 1024.0));
  }
  Table[Nblocks-1] = newp;
  
  /* Update memory tracking */
  TotMem -= SizeTable[Nblocks-1];  /* Remove old size from total */
  TotMem += n;                     /* Add new size to total */
  SizeTable[Nblocks-1] = n;        /* Update size record */
  
  /* Update and potentially report high watermark */
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


/**
 * @brief   Frees memory while enforcing LIFO constraint
 *
 * @param   p       Pointer to memory block to be freed (must be last allocated)
 *
 * This function frees memory while:
 * 1. Enforcing LIFO constraint (can only free the most recently allocated block)
 * 2. Updating memory tracking information
 *
 * The LIFO constraint is strictly enforced - attempting to free any block
 * other than the most recently allocated will result in a fatal error.
 * This prevents memory management bugs but also restricts usage patterns.
 */
void myfree(void *p)
{
  /* Ensure we have blocks to free */
  assert(Nblocks > 0);

  /* Enforce LIFO constraint - can only free the most recently allocated block */
  if(p != Table[Nblocks - 1])
  {
    FATAL_ERROR("Memory management violation: Wrong call of myfree() - pointer %p is not the last allocated block (%p)",
                p, Table[Nblocks-1]);
  }

  /* Free the memory */
  free(p);

  /* Update tracking information */
  Nblocks -= 1;
  TotMem -= SizeTable[Nblocks];  /* Update total memory usage */
}



/**
 * @brief   Prints current memory allocation information
 *
 * This function reports the current total memory allocation in a
 * human-readable format (MB). It's useful for debugging and monitoring
 * memory usage during program execution.
 */
void print_allocated(void)
{
  INFO_LOG("Memory currently allocated: %.2f MB", TotMem / (1024.0 * 1024.0));
}

