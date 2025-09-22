/**
 * @file    io_manager.c
 * @brief   Implementation of I/O abstraction layer
 *
 * This file implements the I/O manager abstraction layer that provides unified
 * access to different I/O formats (binary, HDF5). It serves as the foundation
 * for the future module-aware I/O system while maintaining compatibility with
 * existing implementations.
 *
 * The implementation wraps existing I/O functions and provides a function pointer
 * interface that can be easily extended in future phases to support module-aware
 * output and hierarchical file formats.
 *
 * @author  SAGE Development Team
 * @date    2024
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "io_manager.h"
#include "error.h"
#include "tree.h"
#include "save_binary.h"
#ifdef HDF5
#include "save_hdf5.h"
#include "tree_hdf5.h"
#endif
#include "tree_binary.h"

/* Forward declarations for wrapper functions */
static int wrapper_load_tree_table_binary(int filenr, enum Valid_TreeTypes tree_type);
static int wrapper_load_tree_binary(int filenr, int treenr, enum Valid_TreeTypes tree_type);
static int wrapper_save_galaxies_binary(int filenr, int tree);
static int wrapper_finalize_output_binary(int filenr);

#ifdef HDF5
static int wrapper_load_tree_table_hdf5(int filenr, enum Valid_TreeTypes tree_type);
static int wrapper_load_tree_hdf5(int filenr, int treenr, enum Valid_TreeTypes tree_type);
static int wrapper_save_galaxies_hdf5(int filenr, int tree);
static int wrapper_finalize_output_hdf5(int filenr);
#endif

/**
 * @brief   Initialize I/O manager with format-specific function pointers
 *
 * This function assigns the appropriate function pointers based on the tree type.
 * It creates wrapper functions that adapt the current void-returning I/O functions
 * to the int-returning abstraction layer interface, providing better error handling
 * for future development.
 *
 * @param   manager     Pointer to io_manager_t structure to initialize
 * @param   tree_type   Type of merger tree format to use
 * @return  0 on success, -1 on error
 */
int io_manager_init(io_manager_t *manager, enum Valid_TreeTypes tree_type) {
    if (manager == NULL) {
        ERROR_LOG("io_manager_init: manager parameter is NULL");
        return -1;
    }

    /* Initialize context pointer for future module-aware functionality */
    manager->context = NULL;

    /* Assign function pointers based on tree type */
    switch (tree_type) {
        case lhalo_binary:
            DEBUG_LOG("Initializing I/O manager for binary format");
            manager->load_tree_table = wrapper_load_tree_table_binary;
            manager->load_tree = wrapper_load_tree_binary;
            manager->save_galaxies = wrapper_save_galaxies_binary;
            manager->finalize_output = wrapper_finalize_output_binary;
            break;

#ifdef HDF5
        case genesis_lhalo_hdf5:
            DEBUG_LOG("Initializing I/O manager for HDF5 format");
            manager->load_tree_table = wrapper_load_tree_table_hdf5;
            manager->load_tree = wrapper_load_tree_hdf5;
            manager->save_galaxies = wrapper_save_galaxies_hdf5;
            manager->finalize_output = wrapper_finalize_output_hdf5;
            break;
#endif

        default:
            ERROR_LOG("io_manager_init: Unsupported tree type %d", (int)tree_type);
            return -1;
    }

    DEBUG_LOG("I/O manager initialized successfully for tree type %d", (int)tree_type);
    return 0;
}

/**
 * @brief   Clean up I/O manager resources
 *
 * This function performs cleanup of the I/O manager structure by resetting
 * all function pointers to NULL. This provides safety against accidental
 * use after cleanup and establishes a pattern for future resource management.
 *
 * @param   manager     Pointer to io_manager_t structure to cleanup
 */
void io_manager_cleanup(io_manager_t *manager) {
    if (manager == NULL) {
        return;
    }

    /* Reset function pointers for safety */
    manager->load_tree_table = NULL;
    manager->load_tree = NULL;
    manager->save_galaxies = NULL;
    manager->finalize_output = NULL;
    manager->context = NULL;

    DEBUG_LOG("I/O manager cleaned up");
}

/*
 * Binary format wrapper functions
 *
 * These functions wrap the existing binary I/O functions to provide the
 * int-returning interface required by the abstraction layer while maintaining
 * compatibility with the current void-returning implementations.
 *
 * KNOWN LIMITATION: Error propagation is currently limited because the
 * underlying legacy I/O functions return void and do not provide error codes.
 * These wrappers assume success and return 0. This limitation will be addressed
 * in Phase 5 when the legacy I/O functions are refactored to support proper
 * error reporting through the abstraction layer.
 */

/**
 * @brief   Wrapper for binary format tree table loading
 */
static int wrapper_load_tree_table_binary(int filenr, enum Valid_TreeTypes tree_type) {
    DEBUG_LOG("Loading tree table (binary format) for file %d", filenr);

    /* Call existing binary implementation - currently void return */
    load_tree_table(filenr, tree_type);

    /* The existing function doesn't return error codes, so we assume success */
    /* In Phase 5, this will be enhanced with proper error handling */
    return 0;
}

/**
 * @brief   Wrapper for binary format tree loading
 */
static int wrapper_load_tree_binary(int filenr, int treenr, enum Valid_TreeTypes tree_type) {
    DEBUG_LOG("Loading tree %d from file %d (binary format)", treenr, filenr);

    /* Call existing binary implementation - currently void return */
    load_tree(filenr, treenr, tree_type);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}

/**
 * @brief   Wrapper for binary format galaxy saving
 */
static int wrapper_save_galaxies_binary(int filenr, int tree) {
    DEBUG_LOG("Saving galaxies for tree %d, file %d (binary format)", tree, filenr);

    /* Call existing binary implementation - currently void return */
    save_galaxies(filenr, tree);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}

/**
 * @brief   Wrapper for binary format output finalization
 */
static int wrapper_finalize_output_binary(int filenr) {
    DEBUG_LOG("Finalizing output for file %d (binary format)", filenr);

    /* Call existing binary implementation - currently void return */
    finalize_galaxy_file(filenr);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}

#ifdef HDF5
/*
 * HDF5 format wrapper functions
 *
 * These functions provide the same wrapping functionality for HDF5 format
 * as the binary wrappers above. They prepare the foundation for the future
 * hierarchical HDF5 output system planned in Phase 5.
 */

/**
 * @brief   Wrapper for HDF5 format tree table loading
 */
static int wrapper_load_tree_table_hdf5(int filenr, enum Valid_TreeTypes tree_type) {
    DEBUG_LOG("Loading tree table (HDF5 format) for file %d", filenr);

    /* Call existing HDF5 implementation - currently void return */
    load_tree_table(filenr, tree_type);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}

/**
 * @brief   Wrapper for HDF5 format tree loading
 */
static int wrapper_load_tree_hdf5(int filenr, int treenr, enum Valid_TreeTypes tree_type) {
    DEBUG_LOG("Loading tree %d from file %d (HDF5 format)", treenr, filenr);

    /* Call existing HDF5 implementation - currently void return */
    load_tree(filenr, treenr, tree_type);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}

/**
 * @brief   Wrapper for HDF5 format galaxy saving
 */
static int wrapper_save_galaxies_hdf5(int filenr, int tree) {
    DEBUG_LOG("Saving galaxies for tree %d, file %d (HDF5 format)", tree, filenr);

    /* Call existing HDF5 implementation - currently void return */
    save_galaxies(filenr, tree);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}

/**
 * @brief   Wrapper for HDF5 format output finalization
 */
static int wrapper_finalize_output_hdf5(int filenr) {
    DEBUG_LOG("Finalizing output for file %d (HDF5 format)", filenr);

    /* Call existing HDF5 implementation - currently void return */
    finalize_galaxy_file(filenr);

    /* The existing function doesn't return error codes, so we assume success */
    return 0;
}
#endif /* HDF5 */