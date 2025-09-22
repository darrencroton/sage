/**
 * @file    io_manager.h
 * @brief   I/O abstraction layer for format-agnostic tree input and galaxy output
 *
 * This header defines the I/O manager abstraction layer that provides a unified
 * interface for different input/output formats (binary, HDF5). This abstraction
 * layer serves as the foundation for the future module-aware, format-agnostic
 * I/O system planned for Phase 5.
 *
 * Key features:
 * - Format-agnostic function pointer interface
 * - Runtime format selection based on configuration
 * - Extensible design to support future module-aware output
 * - Maintains compatibility with existing I/O implementations
 *
 * This implementation establishes the architectural foundation required for:
 * - Phase 2A: Integration with physics module interface
 * - Phase 2B: Property-based I/O operations
 * - Phase 5: Full module-aware, hierarchical I/O system
 *
 * @author  SAGE Development Team
 * @date    2024
 */

#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include "config.h"
#include "types.h"

/**
 * @brief   I/O manager structure containing function pointers for all I/O operations
 *
 * This structure provides a unified interface to different I/O formats through
 * function pointers. The specific implementations (binary, HDF5) are assigned
 * at initialization time based on the configuration.
 *
 * Function pointer signatures match existing I/O function interfaces to ensure
 * seamless integration with current codebase while providing foundation for
 * future extensibility.
 *
 * @note    The context pointer is reserved for future module-aware functionality
 *          and is currently unused but ensures API stability for Phase 5 upgrade.
 */
typedef struct {
    /**
     * @brief   Context pointer for future module-aware I/O operations
     *
     * This pointer is reserved for future use when the I/O system becomes
     * module-aware (Phase 5). It will contain information about loaded modules
     * and their properties to enable dynamic output schema adaptation.
     *
     * Currently unused but included for future API compatibility.
     */
    void *context;

    /**
     * @brief   Load merger tree metadata and prepare output files
     *
     * @param   filenr      File number to process
     * @param   tree_type   Type of merger tree format
     * @return  0 on success, non-zero on error
     */
    int (*load_tree_table)(int filenr, enum Valid_TreeTypes tree_type);

    /**
     * @brief   Load a specific merger tree into memory
     *
     * @param   filenr      File number containing the tree
     * @param   treenr      Tree number within the file
     * @param   tree_type   Type of merger tree format
     * @return  0 on success, non-zero on error
     */
    int (*load_tree)(int filenr, int treenr, enum Valid_TreeTypes tree_type);

    /**
     * @brief   Save processed galaxies to output files
     *
     * @param   filenr      File number for output
     * @param   tree        Tree number being processed
     * @return  0 on success, non-zero on error
     */
    int (*save_galaxies)(int filenr, int tree);

    /**
     * @brief   Finalize output files and perform cleanup
     *
     * @param   filenr      File number to finalize
     * @return  0 on success, non-zero on error
     */
    int (*finalize_output)(int filenr);

} io_manager_t;

/**
 * @brief   Initialize I/O manager with appropriate function pointers
 *
 * This function initializes the I/O manager structure by assigning the
 * appropriate function pointers based on the tree type configuration.
 * It serves as the central dispatcher that connects the abstract interface
 * to concrete implementations.
 *
 * @param   manager     Pointer to io_manager_t structure to initialize
 * @param   tree_type   Type of merger tree format to use
 * @return  0 on success, non-zero on error
 *
 * Supported tree types:
 * - lhalo_binary: Binary format (LHalo)
 * - genesis_lhalo_hdf5: HDF5 format (Genesis) - if compiled with HDF5 support
 *
 * @note    This function must be called before using any other I/O manager functions
 * @warning Calling I/O manager functions without initialization results in undefined behavior
 */
int io_manager_init(io_manager_t *manager, enum Valid_TreeTypes tree_type);

/**
 * @brief   Clean up I/O manager resources
 *
 * This function performs any necessary cleanup of I/O manager resources.
 * Currently it resets function pointers to NULL for safety, but may be
 * extended in future phases to handle more complex cleanup scenarios.
 *
 * @param   manager     Pointer to io_manager_t structure to cleanup
 */
void io_manager_cleanup(io_manager_t *manager);

#endif /* IO_MANAGER_H */