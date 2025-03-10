/**
 * @file    util_memory.c
 * @brief   Flexible memory management system with tracking
 *
 * This file implements a memory allocation and tracking system that:
 * - Tracks memory usage and reports high watermarks
 * - Allows memory to be freed in any order (not restricted to LIFO)
 * - Supports configurable limits on tracked blocks
 * - Provides memory leak detection
 *
 * Key functions:
 * - init_memory_system(): Initializes the memory tracking system
 * - mymalloc(): Allocates memory with tracking
 * - myrealloc(): Reallocates memory (any tracked block)
 * - myfree(): Frees memory (any tracked block)
 * - print_allocated(): Reports current memory usage
 * - check_memory_leaks(): Detects and reports memory leaks
 * - cleanup_memory_system(): Frees tracking resources
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "config.h"
#include "core_proto.h"
#include "util_memory.h"

/* Memory tracking variables */
static unsigned long MaxBlocks = DEFAULT_MAX_MEMORY_BLOCKS;
static unsigned long Nblocks = 0;            /* Number of allocated blocks */
static void *(*Table) = NULL;                /* Pointers to allocated blocks */
static size_t (*SizeTable) = NULL;           /* Sizes of allocated blocks */
static size_t TotMem = 0;                    /* Total allocated memory */
static size_t HighMarkMem = 0;               /* High watermark of memory usage */
static size_t OldPrintedHighMark = 0;        /* Last reported high watermark */
static int MemorySystemInitialized = 0;      /* Flag to track initialization state */

/**
 * @brief   Initialize the memory management system
 *
 * @param   max_blocks  Maximum number of memory blocks to track (0 for default)
 * 
 * This function must be called before any other memory functions.
 * It allocates the tracking arrays and initializes the system state.
 */
void init_memory_system(unsigned long max_blocks) {
    /* Use provided max_blocks or default if 0 */
    MaxBlocks = max_blocks > 0 ? max_blocks : DEFAULT_MAX_MEMORY_BLOCKS;
    
    /* Allocate tracking arrays */
    Table = calloc(MaxBlocks, sizeof(void*));
    SizeTable = calloc(MaxBlocks, sizeof(size_t));
    
    if (!Table || !SizeTable) {
        FATAL_ERROR("Memory management system initialization failed: Unable to allocate tracking arrays");
    }
    
    /* Initialize tracking variables */
    Nblocks = 0;
    TotMem = 0;
    HighMarkMem = 0;
    OldPrintedHighMark = 0;
    MemorySystemInitialized = 1;
    
    INFO_LOG("Memory management system initialized with capacity for %lu blocks", MaxBlocks);
}

/**
 * @brief   Find the index of a pointer in the tracking table
 *
 * @param   ptr     Pointer to find
 * @return  Index of the pointer if found, -1 if not found
 * 
 * This helper function first checks the most recently allocated block
 * as a fast path, then performs a linear search if needed.
 */
static int find_block_index(void *ptr) {
    /* Handle NULL pointer case */
    if (ptr == NULL) {
        return -1;
    }
    
    /* Fast path: check if it's the most recently allocated block */
    if (Nblocks > 0 && Table[Nblocks - 1] == ptr) {
        return Nblocks - 1;
    }
    
    /* Standard linear search for other cases */
    for (int i = 0; i < Nblocks; i++) {
        if (Table[i] == ptr) {
            return i;
        }
    }
    return -1;  /* Not found */
}



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
 * 4. Enforcing the maximum block limit (MaxBlocks)
 * 
 * Memory allocated with mymalloc() must be freed with myfree().
 */
void *mymalloc(size_t n)
{
    /* Initialize system if not already done */
    if (!MemorySystemInitialized) {
        init_memory_system(0);  /* Use default block limit */
    }
    
    /* Align memory to 8-byte boundaries for performance */
    if((n % 8) > 0)
        n = (n / 8 + 1) * 8;

    /* Ensure minimum allocation size */
    if(n == 0)
        n = 8;

    /* Check if we've reached the maximum number of tracked blocks */
    if(Nblocks >= MaxBlocks)
    {
        FATAL_ERROR("Memory allocation limit reached: No blocks left in mymalloc(). Increase DEFAULT_MAX_MEMORY_BLOCKS (%lu).", MaxBlocks);
    }

    /* Attempt actual allocation */
    void *ptr = malloc(n);
    if(!ptr)
    {
        FATAL_ERROR("Memory allocation failed: Unable to allocate %.2f MB", n / (1024.0 * 1024.0));
    }

    /* Record allocation */
    Table[Nblocks] = ptr;
    SizeTable[Nblocks] = n;
    
    /* Record allocation size and update total */
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

    /* Increment block count and return pointer */
    Nblocks += 1;
    return ptr;
}


/**
 * @brief   Reallocates memory - no longer restricted to LIFO order
 *
 * @param   p       Pointer to memory block to be reallocated
 * @param   n       New size in bytes
 * @return  Pointer to the reallocated memory block
 *
 * This function reallocates memory while:
 * 1. Finding the block in the tracking table
 * 2. Updating tracking information
 * 3. Handling pointer changes if realloc moves the memory
 *
 * Unlike the original implementation, this function allows reallocating any
 * tracked block, not just the most recently allocated.
 */
void *myrealloc(void *p, size_t n)
{
    /* Handle NULL pointer case */
    if(p == NULL) return mymalloc(n);
    
    /* Initialize system if not already done */
    if (!MemorySystemInitialized) {
        init_memory_system(0);  /* Use default block limit */
    }
    
    /* Align memory to 8-byte boundaries */
    if((n % 8) > 0)
        n = (n / 8 + 1) * 8;
    
    /* Ensure minimum allocation size */
    if(n == 0)
        n = 8;
    
    /* Find the block in the table */
    int index = find_block_index(p);
    if(index == -1)
    {
        FATAL_ERROR("Memory management error: Attempting to reallocate untracked pointer %p", p);
    }
    
    /* Update memory tracking */
    TotMem -= SizeTable[index];  /* Remove old size from total */
    
    /* Attempt reallocation */
    void *newp = realloc(p, n);
    if(newp == NULL)
    {
        FATAL_ERROR("Memory reallocation failed: Unable to reallocate %.2f MB", n / (1024.0 * 1024.0));
    }
    
    /* Update tracking if pointer changed */
    if(newp != p)
    {
        Table[index] = newp;
    }
    
    /* Update size and totals */
    SizeTable[index] = n;
    TotMem += n;
    
    /* Update high watermark if needed */
    if(TotMem > HighMarkMem)
    {
        HighMarkMem = TotMem;
        if(HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0)
        {
            INFO_LOG("New memory usage high mark: %.2f MB", HighMarkMem / (1024.0 * 1024.0));
            OldPrintedHighMark = HighMarkMem;
        }
    }
    
    return newp;
}


/**
 * @brief   Frees memory - no longer restricted to LIFO order
 *
 * @param   p       Pointer to memory block to be freed
 *
 * This function frees memory while:
 * 1. Finding the block in the tracking table
 * 2. Updating memory tracking information
 * 3. Removing the block from tracking
 *
 * Unlike the original implementation, this function allows freeing blocks
 * in any order, not just the most recently allocated.
 */
void myfree(void *p)
{
    /* Handle NULL pointer gracefully */
    if(p == NULL) return;
    
    /* Ensure we have blocks to free */
    if(Nblocks == 0)
    {
        FATAL_ERROR("Memory management error: Attempting to free a block when no blocks are allocated");
    }
    
    /* Find the block in the table */
    int index = find_block_index(p);
    if(index == -1)
    {
        FATAL_ERROR("Memory management error: Attempting to free untracked pointer %p", p);
    }

    /* Optional LIFO violation detection for debugging */
#ifdef DEBUG_LIFO_VIOLATIONS
    if(index != Nblocks - 1)
    {
        WARNING_LOG("LIFO violation: Freeing block at index %d when last allocated is at index %d", 
                    index, Nblocks - 1);
    }
#endif
    
    /* Free the memory */
    free(p);
    
    /* Update tracking information */
    size_t freedSize = SizeTable[index];
    TotMem -= freedSize;
    
    /* Remove from tracking by moving the last element to the freed slot */
    if(index < Nblocks - 1)
    {
        Table[index] = Table[Nblocks - 1];
        SizeTable[index] = SizeTable[Nblocks - 1];
    }
    
    /* Decrement block count */
    Nblocks--;
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
    INFO_LOG("Memory currently allocated: %.2f MB (%lu blocks)", 
             TotMem / (1024.0 * 1024.0), Nblocks);
}

/**
 * @brief   Checks for memory leaks
 * 
 * This function reports any memory blocks that are still allocated,
 * which may indicate memory leaks. It should be called before program
 * termination or during debugging.
 */
void check_memory_leaks(void)
{
    if(Nblocks > 0)
    {
        WARNING_LOG("Memory leak detected: %lu blocks (%.2f MB) still allocated", 
                   Nblocks, TotMem / (1024.0 * 1024.0));
        
        /* Print details of leaked blocks */
        for(unsigned long i = 0; i < Nblocks; i++)
        {
            WARNING_LOG("  Leaked block %lu: %.2f KB at %p", 
                       i, SizeTable[i] / 1024.0, Table[i]);
        }
    }
    else
    {
        INFO_LOG("No memory leaks detected");
    }
}

/**
 * @brief   Clean up the memory tracking system
 * 
 * This function frees the tracking arrays used by the memory system.
 * It should be called at program termination after check_memory_leaks().
 */
void cleanup_memory_system(void)
{
    if (MemorySystemInitialized)
    {
        free(Table);
        free(SizeTable);
        Table = NULL;
        SizeTable = NULL;
        MemorySystemInitialized = 0;
    }
}
