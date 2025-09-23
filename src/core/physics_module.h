/**
 * @file    physics_module.h
 * @brief   Physics module interface for modular SAGE architecture
 *
 * This header defines the core interface for physics modules in SAGE, enabling
 * a physics-agnostic core that can conditionally execute different physics
 * implementations at runtime. This interface establishes the foundation for
 * Principle 1 compliance (Physics-Agnostic Core Infrastructure).
 *
 * Key Components:
 * - physics_module_t: Core module structure with lifecycle functions
 * - Execution phases: Standardized phases matching evolution pipeline
 * - Capability system: Declarative module capabilities and dependencies
 * - Registration system: Dynamic module registration and lookup
 *
 * Design Principles:
 * - Minimal viable interface for Task 2A.1
 * - Foundation for future core/physics separation (Task 2A.2)
 * - Thread-safe module management
 * - Memory-safe using SAGE's centralized memory system
 * - Comprehensive error handling and validation
 *
 * References:
 * - log/sage-master-plan.md: Phase 2A implementation details
 * - log/phase.md: Current task requirements
 * - log/architecture.md: Current codebase analysis
 */

#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H

#include <stdbool.h>
#include <stdint.h>

/* Maximum limits for module system */
#define MAX_MODULE_NAME_LEN     64     /* Maximum module name length */
#define MAX_MODULE_VERSION_LEN  16     /* Maximum version string length */
#define MAX_DEPENDENCY_LEN      256    /* Maximum dependency string length */
#define MAX_REGISTERED_MODULES  32     /* Maximum number of registered modules */

/**
 * @brief Physics execution phases aligned with current evolution pipeline
 *
 * These phases correspond to the main physics processes in the current
 * evolution.c implementation and will enable the core to conditionally
 * execute physics based on loaded modules.
 */
typedef enum {
    PHYSICS_PHASE_INIT         = 0x01,  /* Module initialization */
    PHYSICS_PHASE_INFALL       = 0x02,  /* Cosmological gas infall */
    PHYSICS_PHASE_COOLING      = 0x04,  /* Gas cooling processes */
    PHYSICS_PHASE_STAR_FORMATION = 0x08, /* Star formation and feedback */
    PHYSICS_PHASE_MERGERS      = 0x10,  /* Galaxy mergers and interactions */
    PHYSICS_PHASE_MISC         = 0x20,  /* Miscellaneous processes (stripping, disruption) */
    PHYSICS_PHASE_CLEANUP      = 0x40   /* Module cleanup */
} physics_execution_phase_t;

/**
 * @brief Module capability flags for declarative functionality
 *
 * Modules declare their capabilities using these flags, enabling the core
 * to determine which physics processes are available at runtime.
 */
typedef enum {
    CAPABILITY_NONE            = 0x00,  /* No capabilities (for testing) */
    CAPABILITY_INFALL          = 0x01,  /* Provides cosmological infall */
    CAPABILITY_COOLING         = 0x02,  /* Provides gas cooling */
    CAPABILITY_STAR_FORMATION  = 0x04,  /* Provides star formation */
    CAPABILITY_FEEDBACK        = 0x08,  /* Provides stellar feedback */
    CAPABILITY_MERGERS         = 0x10,  /* Provides merger handling */
    CAPABILITY_DISK_INSTABILITY = 0x20, /* Provides disk instability */
    CAPABILITY_REINCORPORATION = 0x40,  /* Provides gas reincorporation */
    CAPABILITY_AGN             = 0x80   /* Provides AGN physics */
} physics_capability_t;

/**
 * @brief Module execution context for physics functions
 *
 * This structure provides standardized context information to physics
 * modules during execution, enabling access to simulation state and
 * galaxy data without direct coupling to global variables.
 */
typedef struct {
    /* Galaxy indices for current operation */
    int galaxy_index;           /* Index of galaxy being processed */
    int central_galaxy_index;   /* Index of central galaxy */
    int halo_index;             /* Index of current halo */

    /* Time integration context */
    double current_time;        /* Current simulation time */
    double time_step;           /* Current time step size */
    int integration_step;       /* Current integration sub-step */

    /* Simulation parameters (read-only access) */
    const void *config;         /* Pointer to simulation configuration */
    const void *sim_state;      /* Pointer to simulation state */

    /* Future extension points */
    void *reserved1;            /* Reserved for future use */
    void *reserved2;            /* Reserved for future use */
} physics_execution_context_t;

/**
 * @brief Core physics module structure
 *
 * This structure defines the interface that all physics modules must
 * implement. It provides lifecycle management, capability declarations,
 * and execution functions for physics processes.
 */
typedef struct {
    /* Module identification */
    char name[MAX_MODULE_NAME_LEN];         /* Unique module name */
    char version[MAX_MODULE_VERSION_LEN];   /* Module version string */
    char description[128];                  /* Brief module description */

    /* Module lifecycle functions */
    int (*init)(void);                      /* Initialize module */
    int (*execute)(physics_execution_phase_t phase,
                   physics_execution_context_t *context); /* Execute physics */
    int (*cleanup)(void);                   /* Cleanup module resources */

    /* Module capabilities and dependencies */
    uint32_t capabilities;                  /* Capability flags (OR of physics_capability_t) */
    char dependencies[MAX_DEPENDENCY_LEN];  /* Comma-separated dependency list */
    int priority;                           /* Execution priority (lower = earlier) */

    /* Module runtime state */
    bool initialized;                       /* Module initialization status */
    bool active;                           /* Module active status */
    void *private_data;                    /* Module-specific private data */

    /* Future extension points */
    void *reserved1;                       /* Reserved for future use */
    void *reserved2;                       /* Reserved for future use */
} physics_module_t;

/**
 * @brief Module registration and management functions
 *
 * These functions provide the core interface for registering, finding,
 * and managing physics modules at runtime.
 */

/**
 * @brief Register a physics module with the system
 *
 * @param module    Pointer to initialized physics_module_t structure
 * @return          0 on success, negative on error
 */
int register_physics_module(physics_module_t *module);

/**
 * @brief Find a registered physics module by name
 *
 * @param name      Module name to search for
 * @return          Pointer to module if found, NULL otherwise
 */
physics_module_t *find_physics_module(const char *name);

/**
 * @brief Unregister a physics module from the system
 *
 * @param name      Name of module to unregister
 * @return          0 on success, negative on error
 */
int unregister_physics_module(const char *name);

/**
 * @brief Get list of all registered modules
 *
 * @param modules   Array to store module pointers (caller-allocated)
 * @param max_count Maximum number of modules to return
 * @return          Number of modules returned, negative on error
 */
int get_registered_modules(physics_module_t **modules, int max_count);

/**
 * @brief Initialize the physics module system
 *
 * @return          0 on success, negative on error
 */
int initialize_physics_module_system(void);

/**
 * @brief Cleanup the physics module system
 *
 * This function unregisters all modules and frees system resources.
 * Should be called during program shutdown.
 */
void cleanup_physics_module_system(void);

/**
 * @brief Module lifecycle management functions
 *
 * These functions provide centralized management of module lifecycles,
 * ensuring proper initialization order based on dependencies.
 */

/**
 * @brief Initialize all registered modules
 *
 * @return          0 on success, negative on error
 */
int initialize_all_modules(void);

/**
 * @brief Execute a specific physics phase across all capable modules
 *
 * @param phase     Physics phase to execute
 * @param context   Execution context for the phase
 * @return          0 on success, negative on error
 */
int execute_physics_phase(physics_execution_phase_t phase,
                         physics_execution_context_t *context);

/**
 * @brief Cleanup all registered modules
 *
 * @return          0 on success, negative on error
 */
int cleanup_all_modules(void);

/**
 * @brief Module capability and dependency functions
 *
 * These functions provide introspection and validation of module
 * capabilities and dependencies.
 */

/**
 * @brief Check if a module has specific capabilities
 *
 * @param module        Module to check
 * @param capabilities  Capability flags to test (OR of physics_capability_t)
 * @return              true if module has all specified capabilities
 */
bool module_has_capabilities(const physics_module_t *module, uint32_t capabilities);

/**
 * @brief Get modules that provide specific capabilities
 *
 * @param capabilities  Required capability flags
 * @param modules       Array to store matching module pointers
 * @param max_count     Maximum number of modules to return
 * @return              Number of matching modules, negative on error
 */
int get_modules_with_capabilities(uint32_t capabilities,
                                  physics_module_t **modules, int max_count);

/**
 * @brief Validate module dependencies
 *
 * @param module        Module to validate
 * @return              0 if dependencies satisfied, negative on error
 */
int validate_module_dependencies(const physics_module_t *module);

/**
 * @brief Utility functions for module system introspection
 */

/**
 * @brief Print information about all registered modules
 */
void print_registered_modules(void);

/**
 * @brief Get string representation of capabilities
 *
 * @param capabilities  Capability flags to convert
 * @param buffer        Buffer to store string (caller-allocated)
 * @param buffer_size   Size of buffer
 * @return              Number of characters written, negative on error
 */
int capabilities_to_string(uint32_t capabilities, char *buffer, size_t buffer_size);

/**
 * @brief Get string representation of execution phase
 *
 * @param phase         Execution phase to convert
 * @return              String representation (static storage)
 */
const char *phase_to_string(physics_execution_phase_t phase);

#endif /* PHYSICS_MODULE_H */