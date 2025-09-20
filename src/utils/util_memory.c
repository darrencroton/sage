/**
 * @file    util_memory.c
 * @brief   Flexible memory management system with tracking
 *
 * This file implements a memory allocation and tracking system that:
 * - Tracks memory usage and reports high watermarks
 * - Allows memory to be freed in any order (not restricted to LIFO)
 * - Supports configurable limits on tracked blocks
 * - Provides memory leak detection
 * - Tracks memory usage by component category
 * - Supports memory integrity validation
 *
 * Key functions:
 * - init_memory_system(): Initializes the memory tracking system
 * - mymalloc(): Allocates memory with tracking
 * - myrealloc(): Reallocates memory (any tracked block)
 * - myfree(): Frees memory (any tracked block)
 * - print_allocated(): Reports current memory usage
 * - check_memory_leaks(): Detects and reports memory leaks
 * - validate_all_memory(): Validates all allocated memory blocks
 * - cleanup_memory_system(): Frees tracking resources
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "core_proto.h"
#include "util_memory.h"

/* Memory category names for reporting */
static const char *CategoryNames[] = {"Unknown", "Galaxies", "Halos",
                                      "Trees",   "I/O",      "Utility"};

/* Memory tracking variables */
static unsigned long MaxBlocks = DEFAULT_MAX_MEMORY_BLOCKS;
static unsigned long Nblocks = 0;             /* Number of allocated blocks */
static void *(*Table) = NULL;                 /* Pointers to allocated blocks */
static size_t(*SizeTable) = NULL;             /* Sizes of allocated blocks */
static MemoryCategory(*CategoryTable) = NULL; /* Category of each block */
static size_t TotMem = 0;                     /* Total allocated memory */
static size_t HighMarkMem = 0;          /* High watermark of memory usage */
static size_t OldPrintedHighMark = 0;   /* Last reported high watermark */
static int MemorySystemInitialized = 0; /* Flag to track initialization state */
static size_t CategorySizes[MEM_MAX_CATEGORY] = {
    0}; /* Memory used per category */
static int MemoryReportLevel =
    MEMORY_REPORT_MINIMAL; /* Default to minimal reporting */

#ifdef DEBUG_MEMORY
#define MEMORY_GUARD_SIZE 8 /* Size of guard areas in bytes */
#define MEMORY_GUARD_VALUE 0xDEADBEEF
static void add_memory_guards(void *real_ptr, size_t size);
static int check_memory_guards(void *user_ptr, size_t size);
#endif

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
  Table = calloc(MaxBlocks, sizeof(void *));
  SizeTable = calloc(MaxBlocks, sizeof(size_t));
  CategoryTable = calloc(MaxBlocks, sizeof(MemoryCategory));

  if (!Table || !SizeTable || !CategoryTable) {
    FATAL_ERROR("Memory management system initialization failed: Unable to "
                "allocate tracking arrays");
  }

  /* Initialize tracking variables */
  Nblocks = 0;
  TotMem = 0;
  HighMarkMem = 0;
  OldPrintedHighMark = 0;
  memset(CategorySizes, 0, sizeof(CategorySizes));
  MemorySystemInitialized = 1;

  INFO_LOG("Memory management system initialized with capacity for %lu blocks",
           MaxBlocks);
}

/**
 * @brief   Set memory reporting level
 *
 * @param   level   Reporting level (NONE, MINIMAL, or DETAILED)
 *
 * This function controls how much information is reported about memory
 * operations. Higher levels provide more detailed information but may
 * impact performance.
 */
void set_memory_reporting(int level) {
  if (level >= MEMORY_REPORT_NONE && level <= MEMORY_REPORT_DETAILED) {
    MemoryReportLevel = level;
  }
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
  return -1; /* Not found */
}

/**
 * @brief   Allocates memory with category tracking
 *
 * @param   size      Number of bytes to allocate
 * @param   category  Memory category for tracking
 * @return  Pointer to the allocated memory block
 *
 * This function allocates memory while tracking its category, allowing
 * for more detailed memory usage analysis. Categories can represent
 * different components of the application (galaxies, halos, etc.).
 */
void *mymalloc_cat(size_t size, MemoryCategory category) {
  /* Initialize system if not already done */
  if (!MemorySystemInitialized) {
    init_memory_system(0); /* Use default block limit */
  }

  /* Validate category */
  if (category < 0 || category >= MEM_MAX_CATEGORY) {
    category = MEM_UNKNOWN;
  }

  /* Align memory to 8-byte boundaries for performance */
  if ((size % 8) > 0)
    size = (size / 8 + 1) * 8;

  /* Ensure minimum allocation size */
  if (size == 0)
    size = 8;

#ifdef DEBUG_MEMORY
  /* Add space for memory guards */
  size_t alloc_size = size + 2 * MEMORY_GUARD_SIZE;
#else
  size_t alloc_size = size;
#endif

  /* Check if we've reached the maximum number of tracked blocks */
  if (Nblocks >= MaxBlocks) {
    FATAL_ERROR("Memory allocation limit reached: No blocks left in "
                "mymalloc(). Increase DEFAULT_MAX_MEMORY_BLOCKS (%lu).",
                MaxBlocks);
  }

  /* Attempt actual allocation */
  void *ptr = malloc(alloc_size);
  if (!ptr) {
    FATAL_ERROR("Memory allocation failed: Unable to allocate %.2f MB",
                alloc_size / (1024.0 * 1024.0));
  }

#ifdef DEBUG_MEMORY
  /* Add memory guards */
  add_memory_guards(ptr, size);

  /* Adjust pointer to skip initial guard */
  void *user_ptr = (char *)ptr + MEMORY_GUARD_SIZE;
#else
  void *user_ptr = ptr;
#endif

  /* Record allocation */
  Table[Nblocks] = user_ptr;
  SizeTable[Nblocks] = size;
  CategoryTable[Nblocks] = category;

  /* Update category statistics */
  CategorySizes[category] += size;

  /* Record allocation size and update total */
  TotMem += size;

  /* Update and potentially report high watermark */
  if (TotMem > HighMarkMem) {
    HighMarkMem = TotMem;
    /* Only report when appropriate based on level */
    if (MemoryReportLevel >= MEMORY_REPORT_MINIMAL &&
        HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0) {
      INFO_LOG("New memory usage high mark: %.2f MB",
               HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  /* Optional detailed reporting */
  if (MemoryReportLevel >= MEMORY_REPORT_DETAILED) {
    INFO_LOG("Allocated %.2f KB for category %s", size / 1024.0,
             CategoryNames[category]);
  }

  /* Increment block count and return pointer */
  Nblocks += 1;
  return user_ptr;
}

/**
 * @brief   Allocates memory with tracking (backward compatibility wrapper)
 *
 * @param   size    Number of bytes to allocate
 * @return  Pointer to the allocated memory block
 *
 * This function is a wrapper around mymalloc_cat() that uses the
 * default UNKNOWN category for backward compatibility.
 */
void *mymalloc(size_t size) { return mymalloc_cat(size, MEM_UNKNOWN); }

/**
 * @brief   Reallocates memory with category tracking
 *
 * @param   p         Pointer to memory block to be reallocated
 * @param   size      New size in bytes
 * @param   category  Memory category for tracking
 * @return  Pointer to the reallocated memory block
 *
 * This function reallocates memory while updating its category tracking.
 * It handles memory guards if enabled and updates all tracking information.
 */
void *myrealloc_cat(void *p, size_t size, MemoryCategory category) {
  /* Handle NULL pointer case */
  if (p == NULL)
    return mymalloc_cat(size, category);

  /* Validate category */
  if (category < 0 || category >= MEM_MAX_CATEGORY) {
    category = MEM_UNKNOWN;
  }

  /* Align memory to 8-byte boundaries */
  if ((size % 8) > 0)
    size = (size / 8 + 1) * 8;

  /* Ensure minimum allocation size */
  if (size == 0)
    size = 8;

  /* Find the block in the table */
  int index = find_block_index(p);
  if (index == -1) {
    FATAL_ERROR("Memory management error: Attempting to reallocate untracked "
                "pointer %p",
                p);
  }

#ifdef DEBUG_MEMORY
  /* Validate memory guards */
  if (!check_memory_guards(p, SizeTable[index])) {
    FATAL_ERROR("Memory corruption detected: Guards damaged for pointer %p", p);
  }

  /* Calculate the real pointer with guard offset */
  void *real_ptr = (char *)p - MEMORY_GUARD_SIZE;

  /* Calculate new size with guards */
  size_t alloc_size = size + 2 * MEMORY_GUARD_SIZE;
#else
  void *real_ptr = p;
  size_t alloc_size = size;
#endif

  /* Update category statistics */
  CategorySizes[CategoryTable[index]] -= SizeTable[index];
  CategorySizes[category] += size;

  /* Update memory tracking */
  TotMem -= SizeTable[index]; /* Remove old size from total */

  /* Attempt reallocation */
  void *newp = realloc(real_ptr, alloc_size);
  if (newp == NULL) {
    FATAL_ERROR("Memory reallocation failed: Unable to reallocate %.2f MB",
                alloc_size / (1024.0 * 1024.0));
  }

#ifdef DEBUG_MEMORY
  /* Add/update memory guards */
  add_memory_guards(newp, size);

  /* Adjust pointer to skip initial guard */
  void *user_newp = (char *)newp + MEMORY_GUARD_SIZE;
#else
  void *user_newp = newp;
#endif

  /* Update tracking if pointer changed */
  if (user_newp != p) {
    Table[index] = user_newp;
  }

  /* Update size, category, and totals */
  SizeTable[index] = size;
  CategoryTable[index] = category;
  TotMem += size;

  /* Update high watermark if needed */
  if (TotMem > HighMarkMem) {
    HighMarkMem = TotMem;
    if (MemoryReportLevel >= MEMORY_REPORT_MINIMAL &&
        HighMarkMem > OldPrintedHighMark + 10 * 1024.0 * 1024.0) {
      INFO_LOG("New memory usage high mark: %.2f MB",
               HighMarkMem / (1024.0 * 1024.0));
      OldPrintedHighMark = HighMarkMem;
    }
  }

  /* Optional detailed reporting */
  if (MemoryReportLevel >= MEMORY_REPORT_DETAILED) {
    INFO_LOG("Reallocated %.2f KB for category %s", size / 1024.0,
             CategoryNames[category]);
  }

  return user_newp;
}

/**
 * @brief   Reallocates memory (backward compatibility wrapper)
 *
 * @param   p       Pointer to memory block to be reallocated
 * @param   size    New size in bytes
 * @return  Pointer to the reallocated memory block
 *
 * This function is a wrapper around myrealloc_cat() that preserves
 * the original category for backward compatibility.
 */
void *myrealloc(void *p, size_t size) {
  if (p == NULL)
    return mymalloc(size);

  int index = find_block_index(p);
  if (index == -1) {
    FATAL_ERROR("Memory management error: Attempting to reallocate untracked "
                "pointer %p",
                p);
  }

  /* Keep the original category */
  MemoryCategory category = CategoryTable[index];
  return myrealloc_cat(p, size, category);
}

/**
 * @brief   Frees memory with enhanced checking
 *
 * @param   p       Pointer to memory block to be freed
 *
 * This function frees memory while updating category statistics and
 * validating memory integrity if guards are enabled. It handles
 * removal from tracking arrays and updates totals.
 */
void myfree(void *p) {
  /* Handle NULL pointer gracefully */
  if (p == NULL)
    return;

  /* Ensure we have blocks to free */
  if (Nblocks == 0) {
    FATAL_ERROR("Memory management error: Attempting to free a block when no "
                "blocks are allocated");
  }

  /* Find the block in the table */
  int index = find_block_index(p);
  if (index == -1) {
    FATAL_ERROR(
        "Memory management error: Attempting to free untracked pointer %p", p);
  }

#ifdef DEBUG_MEMORY
  /* Validate memory guards */
  if (!check_memory_guards(p, SizeTable[index])) {
    FATAL_ERROR("Memory corruption detected: Guards damaged for pointer %p", p);
  }

  /* Calculate the real pointer with guard offset */
  void *real_ptr = (char *)p - MEMORY_GUARD_SIZE;
#else
  void *real_ptr = p;
#endif

  /* Update category statistics */
  CategorySizes[CategoryTable[index]] -= SizeTable[index];

  /* Optional detailed reporting */
  if (MemoryReportLevel >= MEMORY_REPORT_DETAILED) {
    INFO_LOG("Freed %.2f KB from category %s", SizeTable[index] / 1024.0,
             CategoryNames[CategoryTable[index]]);
  }

  /* Update tracking information */
  TotMem -= SizeTable[index];

  /* Free the memory */
  free(real_ptr);

  /* Remove from tracking by moving the last element to the freed slot */
  if (index < Nblocks - 1) {
    Table[index] = Table[Nblocks - 1];
    SizeTable[index] = SizeTable[Nblocks - 1];
    CategoryTable[index] = CategoryTable[Nblocks - 1];
  }

  /* Decrement block count */
  Nblocks--;
}

/**
 * @brief   Print current memory usage (brief version)
 *
 * This function reports current memory usage in a concise format,
 * showing only the most important statistics. It's designed for
 * regular reporting where detailed information is not needed.
 */
void print_memory_brief(void) {
  if (MemorySystemInitialized) {
    INFO_LOG("Memory: %.2f MB used, %.2f MB peak, %lu blocks",
             TotMem / (1024.0 * 1024.0), HighMarkMem / (1024.0 * 1024.0),
             Nblocks);
  }
}

/**
 * @brief   Print current memory allocation information
 *
 * This function reports detailed memory allocation information,
 * optionally including category breakdowns based on the current
 * reporting level.
 */
void print_allocated(void) {
  if (MemorySystemInitialized) {
    INFO_LOG("Memory currently allocated: %.2f MB (%lu blocks)",
             TotMem / (1024.0 * 1024.0), Nblocks);

    if (MemoryReportLevel >= MEMORY_REPORT_DETAILED) {
      print_allocated_by_category();
    }
  }
}

/**
 * @brief   Print memory usage by category
 *
 * This function reports memory usage broken down by category,
 * showing how much memory each component of the application is using.
 */
void print_allocated_by_category(void) {
  if (!MemorySystemInitialized)
    return;

  INFO_LOG("Memory usage by category:");
  for (int i = 0; i < MEM_MAX_CATEGORY; i++) {
    if (CategorySizes[i] > 0) {
      INFO_LOG("  %s: %.2f MB", CategoryNames[i],
               CategorySizes[i] / (1024.0 * 1024.0));
    }
  }
}

/**
 * @brief   Check for memory leaks
 *
 * This function reports any memory blocks that are still allocated,
 * which may indicate memory leaks. It provides categorized information
 * about leaks to help identify the source of the problem.
 */
void check_memory_leaks(void) {
  if (Nblocks > 0) {
    WARNING_LOG("Memory leak detected: %lu blocks (%.2f MB) still allocated",
                Nblocks, TotMem / (1024.0 * 1024.0));

    /* Count leaks by category */
    size_t category_counts[MEM_MAX_CATEGORY] = {0};
    for (unsigned long i = 0; i < Nblocks; i++) {
      category_counts[CategoryTable[i]]++;
    }

    /* Report leaks by category */
    WARNING_LOG("Leaked blocks by category:");
    for (int i = 0; i < MEM_MAX_CATEGORY; i++) {
      if (category_counts[i] > 0) {
        WARNING_LOG("  %s: %zu blocks", CategoryNames[i], category_counts[i]);
      }
    }

    /* Print details of leaked blocks if detailed reporting enabled */
    if (MemoryReportLevel >= MEMORY_REPORT_DETAILED) {
      for (unsigned long i = 0; i < Nblocks; i++) {
        WARNING_LOG("  Leaked block %lu: %.2f KB at %p (category: %s)", i,
                    SizeTable[i] / 1024.0, Table[i],
                    CategoryNames[CategoryTable[i]]);
      }
    }
  } else {
    INFO_LOG("No memory leaks detected");
  }
}

#ifdef DEBUG_MEMORY
/**
 * @brief   Add memory guards around an allocation
 *
 * @param   real_ptr  Pointer to the actual memory block
 * @param   size      Size of the user data area
 *
 * This function adds guard values before and after the user data area
 * to detect buffer overruns and underruns. It's only compiled in debug mode.
 */
static void add_memory_guards(void *real_ptr, size_t size) {
  /* Calculate guard locations */
  uint32_t *front_guard = (uint32_t *)real_ptr;
  uint32_t *back_guard =
      (uint32_t *)((char *)real_ptr + MEMORY_GUARD_SIZE + size);

  /* Set front guard (2 32-bit values) */
  front_guard[0] = MEMORY_GUARD_VALUE;
  front_guard[1] = MEMORY_GUARD_VALUE;

  /* Set back guard (2 32-bit values) */
  back_guard[0] = MEMORY_GUARD_VALUE;
  back_guard[1] = MEMORY_GUARD_VALUE;
}

/**
 * @brief   Check memory guards around an allocation
 *
 * @param   user_ptr  Pointer to the user data area
 * @param   size      Size of the user data area
 * @return  1 if guards are intact, 0 if damaged
 *
 * This function checks guard values before and after the user data area
 * to detect buffer overruns and underruns. It's only compiled in debug mode.
 */
static int check_memory_guards(void *user_ptr, size_t size) {
  /* Calculate guard locations */
  uint32_t *front_guard = (uint32_t *)((char *)user_ptr - MEMORY_GUARD_SIZE);
  uint32_t *back_guard = (uint32_t *)((char *)user_ptr + size);

  /* Check guards */
  if (front_guard[0] != MEMORY_GUARD_VALUE ||
      front_guard[1] != MEMORY_GUARD_VALUE ||
      back_guard[0] != MEMORY_GUARD_VALUE ||
      back_guard[1] != MEMORY_GUARD_VALUE) {
    return 0; /* Guards damaged */
  }

  return 1; /* Guards intact */
}
#endif

/**
 * @brief   Validate a specific memory block
 *
 * @param   ptr     Pointer to the memory block to validate
 * @return  1 if valid, 0 if invalid or not tracked
 *
 * This function checks if a pointer is valid and optionally verifies
 * memory guards if they are enabled. It helps detect corruption in
 * specific blocks.
 */
int validate_memory_block(void *ptr) {
  /* Find the block in the table */
  int index = find_block_index(ptr);
  if (index == -1) {
    return 0; /* Not a tracked block */
  }

#ifdef DEBUG_MEMORY
  /* Check memory guards */
  if (!check_memory_guards(ptr, SizeTable[index])) {
    WARNING_LOG("Memory corruption detected: Guards damaged for pointer %p",
                ptr);
    return 0;
  }
#endif

  return 1; /* Block is valid */
}

/**
 * @brief   Validate all tracked memory blocks
 *
 * @return  1 if all blocks are valid, 0 if any corruption detected
 *
 * This function checks all tracked memory blocks for validity and
 * reports any corrupted blocks. It's useful for detecting memory
 * corruption issues.
 */
int validate_all_memory(void) {
  int valid_blocks = 0;
  int invalid_blocks = 0;

  for (unsigned long i = 0; i < Nblocks; i++) {
#ifdef DEBUG_MEMORY
    if (check_memory_guards(Table[i], SizeTable[i])) {
      valid_blocks++;
    } else {
      WARNING_LOG(
          "Memory corruption detected: Guards damaged for block %lu at %p", i,
          Table[i]);
      invalid_blocks++;
    }
#else
    valid_blocks++; /* Without guards, we can only confirm it's tracked */
#endif
  }

  if (invalid_blocks == 0) {
    if (MemoryReportLevel >= MEMORY_REPORT_MINIMAL) {
      INFO_LOG("All %d memory blocks are valid", valid_blocks);
    }
    return 1;
  } else {
    WARNING_LOG("Memory validation: %d valid blocks, %d corrupted blocks",
                valid_blocks, invalid_blocks);
    return 0;
  }
}

/**
 * @brief   Clean up the memory tracking system
 *
 * This function frees the tracking arrays used by the memory system.
 * It should be called at program termination after check_memory_leaks().
 */
void cleanup_memory_system(void) {
  if (MemorySystemInitialized) {
    if (MemoryReportLevel >= MEMORY_REPORT_MINIMAL) {
      print_memory_brief();
    }

    free(Table);
    free(SizeTable);
    free(CategoryTable);
    Table = NULL;
    SizeTable = NULL;
    CategoryTable = NULL;
    MemorySystemInitialized = 0;
  }
}