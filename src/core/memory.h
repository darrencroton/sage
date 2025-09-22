#ifndef CORE_MEMORY_H
#define CORE_MEMORY_H

/**
 * @file    memory.h
 * @brief   Centralized memory management interface for SAGE
 *
 * This header provides a centralized interface to the SAGE memory management
 * system, ensuring all memory allocations go through the tracking and
 * categorization system in memory.c.
 *
 * Key features:
 * - All allocations tracked with leak detection
 * - Memory categorization for component-level analysis
 * - Bounded memory usage with high-water mark monitoring
 * - Memory integrity validation (when compiled with DEBUG_MEMORY)
 *
 * Usage:
 * Include this header instead of directly including memory.h to ensure
 * consistent memory management throughout the SAGE codebase.
 */

#include "../utils/memory.h"

/*
 * Centralized memory management functions
 * All memory allocations in SAGE should use these functions instead of
 * standard library malloc/calloc/realloc/free to ensure proper tracking
 * and prepare for module-aware memory management.
 */

#endif /* CORE_MEMORY_H */