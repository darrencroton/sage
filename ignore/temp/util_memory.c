/**
 * @file    util_memory.c
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
 * - mymalloc(): Allocates aligned memory with tracking
 * - myfree(): Releases memory with LIFO validation
 * - myrealloc(): Reallocates memory while preserving alignment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "core_allvars.h"
#include "core_proto.h"
#include "util_error.h"

#ifdef DEBUG_MEMORY_USE

#define MAXBLOCKS 5000

static unsigned long Nblocks = 0;
static void *Table[MAXBLOCKS];
static size_t SizeTable[MAXBLOCKS];
static const char *NameTable[MAXBLOCKS];
static const char *FunctionTable[MAXBLOCKS];
static char *FileTable[MAXBLOCKS];
static int LineTable[MAXBLOCKS];
static unsigned long TotMem = 0, HighMarkMem = 0, OldPrintedHighMark = 0;
static unsigned long count_malloc = 0, count_realloc = 0, count_free = 0;

#endif

/**
 * @brief   Allocates memory with alignment and tracking
 *
 * @param   size      Size of memory to allocate in bytes
 * @param   func      Name of the calling function
 * @param   file      Name of the source file
 * @param   line      Line number in the source file
 * @param   msg       Optional message to describe the allocation
 * @return  Pointer to the allocated memory
 * 
 * This function allocates memory with 8-byte alignment for optimal performance.
 * It also tracks all allocations for debugging purposes and to enforce LIFO
 * ordering. When debug tracking is enabled, it keeps records of:
 * - The allocation size
 * - The calling function, file, and line
 * - An optional descriptive message
 * 
 * The function also monitors total memory usage and reports high watermarks
 * when memory usage increases significantly.
 */
void *mymalloc_fullinfo(size_t size, const char *func, const char *file, int line, const char *msg)
{
  void *p;
  int alignment = 8;
  
  // Ensure allocation is aligned to 8 bytes for optimal performance
  if(size % alignment != 0)
    size = (size / alignment + 1) * alignment;

  // Attempt to allocate the memory
  if(!(p = malloc(size)))
  {
    FATAL_ERROR("Failed to allocate %g MB of memory in %s() at %s:%d for '%s'",
               size / (1024.0 * 1024.0), func, file, line, msg);
  }

#ifdef DEBUG_MEMORY_USE
  // Track allocation if debugging is enabled
  count_malloc++;

  // Update total memory usage
  TotMem += size;
  if(TotMem > HighMarkMem)
  {
    HighMarkMem = TotMem;
    // Report significant increases in memory usage
    if(HighMarkMem > OldPrintedHighMark + 10 * 1024 * 1024)
    {
      DEBUG_LOG("High-water mark memory usage: %g MB", HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  // Store allocation details in tracking tables
  if(Nblocks < MAXBLOCKS)
  {
    Table[Nblocks] = p;
    SizeTable[Nblocks] = size;
    NameTable[Nblocks] = msg;
    FunctionTable[Nblocks] = func;
    char *fileCopy = strdup(file);
    if (fileCopy == NULL) {
      FATAL_ERROR("Could not allocate memory for file name in memory tracking system");
    }
    FileTable[Nblocks] = fileCopy;
    LineTable[Nblocks] = line;
    Nblocks++;
  }
  else
  {
    FATAL_ERROR("Too many active memory allocations (>MAXBLOCKS=%d) in memory tracking system", MAXBLOCKS);
  }
#endif

  return p;
}

/**
 * @brief   Releases memory with LIFO validation
 *
 * @param   p         Pointer to memory to free
 * @param   func      Name of the calling function
 * @param   file      Name of the source file
 * @param   line      Line number in the source file
 * @return  0 on success
 * 
 * This function frees memory previously allocated with mymalloc() and updates
 * tracking information. It enforces LIFO ordering by ensuring that memory blocks
 * are freed in exactly the reverse order they were allocated.
 * 
 * The LIFO constraint helps catch memory management bugs early by ensuring a
 * structured approach to allocation and deallocation.
 */
int myfree_fullinfo(void *p, const char *func, const char *file, int line)
{
#ifdef DEBUG_MEMORY_USE
  count_free++;

  // Find the block in the tracking tables
  int i;
  for(i = 0; i < Nblocks; i++)
  {
    if(Table[i] == p)
      break;
  }

  // Check for invalid free operation
  if(i >= Nblocks)
  {
    FATAL_ERROR("Attempt to free memory that wasn't allocated in %s() at %s:%d", func, file, line);
  }

  // Enforce LIFO constraint
  if(p != Table[Nblocks - 1])
  {
    // Find what block should have been freed
    char *shouldbe = (char*)file;
    int j;
    
    // Find the most recently allocated block
    for(j = Nblocks - 1; j >= 0; j--)
    {
      if(Table[j] == p)
        break;
      else
        shouldbe = FileTable[j];
    }

    // Report the LIFO violation
    FATAL_ERROR("Wrong call of myfree() at %s:%d: should have been %s - not respecting LIFO order", file, line, shouldbe);
  }

  // Update tracking information
  Nblocks--;
  TotMem -= SizeTable[i];
  
  // Free the file name copy
  free(FileTable[i]);
#endif

  // Free the memory
  free(p);
  return 0;
}

/**
 * @brief   Reallocates memory with alignment preservation
 *
 * @param   p         Pointer to memory to reallocate
 * @param   size      New size in bytes
 * @param   func      Name of the calling function
 * @param   file      Name of the source file
 * @param   line      Line number in the source file
 * @param   msg       Optional message to describe the reallocation
 * @return  Pointer to the reallocated memory
 * 
 * This function reallocates memory previously allocated with mymalloc() to a
 * new size. It maintains 8-byte alignment for optimal performance and updates
 * tracking information.
 * 
 * The function enforces LIFO ordering by ensuring that only the most recently
 * allocated block can be reallocated.
 */
void *myrealloc_fullinfo(void *p, size_t size, const char *func, const char *file, int line, const char *msg)
{
  void *pnew;
  int alignment = 8;

  // Ensure alignment to 8 bytes
  if(size % alignment != 0)
    size = (size / alignment + 1) * alignment;

#ifdef DEBUG_MEMORY_USE
  count_realloc++;

  // Check for NULL pointer (equivalent to malloc)
  if(p == NULL)
    return mymalloc_fullinfo(size, func, file, line, msg);

  // Find the block in the tracking tables
  int i;
  for(i = 0; i < Nblocks; i++)
  {
    if(Table[i] == p)
      break;
  }

  // Check for invalid reallocation
  if(i >= Nblocks)
  {
    FATAL_ERROR("Attempt to reallocate memory that wasn't allocated in %s() at %s:%d for '%s'",
              func, file, line, msg);
  }

  // Enforce LIFO constraint
  if(p != Table[Nblocks - 1])
  {
    // Find what block should have been reallocated
    char *shouldbe = (char*)file;
    int j;
    
    // Find the most recently allocated block
    for(j = Nblocks - 1; j >= 0; j--)
    {
      if(Table[j] == p)
        break;
      else
        shouldbe = FileTable[j];
    }

    // Report the LIFO violation
    FATAL_ERROR("Wrong call of myrealloc() at %s:%d: should have been %s - not respecting LIFO order", file, line, shouldbe);
  }
#endif

  // Attempt to reallocate the memory
  pnew = realloc(p, size);
  if(!pnew)
  {
    FATAL_ERROR("Failed to reallocate %g MB of memory in %s() at %s:%d for '%s'",
             size / (1024.0 * 1024.0), func, file, line, msg);
  }

#ifdef DEBUG_MEMORY_USE
  // Update tracking information
  Table[i] = pnew;
  TotMem -= SizeTable[i];
  SizeTable[i] = size;
  TotMem += size;
  NameTable[i] = msg;
  FunctionTable[i] = func;
  free(FileTable[i]);
  char *fileCopy = strdup(file);
  if (fileCopy == NULL) {
    FATAL_ERROR("Could not allocate memory for file name in memory tracking system");
  }
  FileTable[i] = fileCopy;
  LineTable[i] = line;

  // Update high watermark if needed
  if(TotMem > HighMarkMem)
  {
    HighMarkMem = TotMem;
    if(HighMarkMem > OldPrintedHighMark + 10 * 1024 * 1024)
    {
      DEBUG_LOG("High-water mark memory usage: %g MB", HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }
#endif

  return pnew;
}

/**
 * @brief   Reports memory usage statistics
 * 
 * This function prints memory usage statistics when DEBUG_MEMORY_USE is defined.
 * It includes information about:
 * - Total memory currently allocated
 * - High watermark memory usage
 * - Number of allocation, reallocation, and free operations
 * - Number of currently active allocations
 */
void report_memory_usage(void)
{
#ifdef DEBUG_MEMORY_USE
  INFO_LOG("Memory usage statistics:");
  INFO_LOG("  Allocated: %g MB in %lu blocks", TotMem / (1024.0 * 1024.0), Nblocks);
  INFO_LOG("  High water mark: %g MB", HighMarkMem / (1024.0 * 1024.0));
  INFO_LOG("  Operations: %lu mallocs, %lu reallocs, %lu frees", count_malloc, count_realloc, count_free);
  
  if(Nblocks > 0)
  {
    INFO_LOG("  Remaining allocations:");
    for(int i = 0; i < Nblocks; i++)
    {
      INFO_LOG("    Block %d: %zu bytes for '%s' allocated in %s() at %s:%d", 
             i, SizeTable[i], NameTable[i], FunctionTable[i], FileTable[i], LineTable[i]);
    }
  }
#endif
}

/**
 * @brief   Simplified allocation function
 *
 * @param   size      Size of memory to allocate in bytes
 * @return  Pointer to the allocated memory
 * 
 * This is a simple wrapper around mymalloc_fullinfo() that provides a basic
 * description for allocations where detailed information is not needed.
 */
void *mymalloc(size_t size)
{
  return mymalloc_fullinfo(size, "unknown", "unknown", 0, "unspecified");
}

/**
 * @brief   Simplified deallocation function
 *
 * @param   p    Pointer to memory to free
 * @return  0 on success
 * 
 * This is a simple wrapper around myfree_fullinfo() for deallocations
 * where detailed information tracking is not needed.
 */
int myfree(void *p)
{
  return myfree_fullinfo(p, "unknown", "unknown", 0);
}
