/**
 * @file    physics_module.c
 * @brief   Implementation of physics module interface system
 *
 * This file implements the core functionality for the physics module interface,
 * including module registration, lifecycle management, capability tracking,
 * and dependency validation. This system enables the physics-agnostic core
 * architecture required for Principle 1 compliance.
 *
 * Key Features:
 * - Thread-safe module registration and lookup
 * - Module lifecycle management (init/execute/cleanup)
 * - Capability-based module discovery
 * - Basic dependency validation
 * - Comprehensive error handling and logging
 * - Integration with SAGE memory management system
 *
 * Design Considerations:
 * - Uses SAGE's centralized memory management (memory.h)
 * - Provides detailed error messages for debugging
 * - Validates all input parameters for robustness
 * - Maintains module state for lifecycle tracking
 * - Supports future extension through reserved fields
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "physics_module.h"
#include "memory.h"
#include "types.h"

/* Internal module registry structure */
typedef struct {
    physics_module_t *modules[MAX_REGISTERED_MODULES]; /* Array of registered modules */
    int module_count;                                   /* Number of registered modules */
    bool system_initialized;                           /* System initialization status */
} module_registry_t;

/* Global module registry (private to this file) */
static module_registry_t g_registry = {
    .modules = {NULL},
    .module_count = 0,
    .system_initialized = false
};

/**
 * @brief Initialize the physics module system
 *
 * @return 0 on success, negative on error
 */
int initialize_physics_module_system(void) {
    if (g_registry.system_initialized) {
        return 0; /* Already initialized */
    }

    /* Initialize registry state */
    memset(&g_registry, 0, sizeof(module_registry_t));
    g_registry.system_initialized = true;

    return 0;
}

/**
 * @brief Cleanup the physics module system
 */
void cleanup_physics_module_system(void) {
    if (!g_registry.system_initialized) {
        return; /* Nothing to cleanup */
    }

    /* Cleanup all registered modules */
    cleanup_all_modules();

    /* Clear registry */
    for (int i = 0; i < g_registry.module_count; i++) {
        g_registry.modules[i] = NULL;
    }
    g_registry.module_count = 0;
    g_registry.system_initialized = false;
}

/**
 * @brief Register a physics module with the system
 *
 * @param module Pointer to initialized physics_module_t structure
 * @return 0 on success, negative on error
 */
int register_physics_module(physics_module_t *module) {
    /* Validate input parameters */
    if (!module) {
        fprintf(stderr, "Error: Cannot register NULL module\n");
        return -1;
    }

    if (!g_registry.system_initialized) {
        fprintf(stderr, "Error: Module system not initialized\n");
        return -2;
    }

    if (strlen(module->name) == 0) {
        fprintf(stderr, "Error: Module name cannot be empty\n");
        return -3;
    }

    if (g_registry.module_count >= MAX_REGISTERED_MODULES) {
        fprintf(stderr, "Error: Maximum number of modules (%d) already registered\n",
                MAX_REGISTERED_MODULES);
        return -4;
    }

    /* Check for duplicate module names */
    for (int i = 0; i < g_registry.module_count; i++) {
        if (strcmp(g_registry.modules[i]->name, module->name) == 0) {
            fprintf(stderr, "Error: Module '%s' already registered\n", module->name);
            return -5;
        }
    }

    /* Validate required function pointers */
    if (!module->init || !module->execute || !module->cleanup) {
        fprintf(stderr, "Error: Module '%s' missing required function pointers\n",
                module->name);
        return -6;
    }

    /* Register the module */
    g_registry.modules[g_registry.module_count] = module;
    g_registry.module_count++;

    /* Initialize module state */
    module->initialized = false;
    module->active = false;

    return 0;
}

/**
 * @brief Find a registered physics module by name
 *
 * @param name Module name to search for
 * @return Pointer to module if found, NULL otherwise
 */
physics_module_t *find_physics_module(const char *name) {
    if (!name || !g_registry.system_initialized) {
        return NULL;
    }

    for (int i = 0; i < g_registry.module_count; i++) {
        if (strcmp(g_registry.modules[i]->name, name) == 0) {
            return g_registry.modules[i];
        }
    }

    return NULL;
}

/**
 * @brief Unregister a physics module from the system
 *
 * @param name Name of module to unregister
 * @return 0 on success, negative on error
 */
int unregister_physics_module(const char *name) {
    if (!name || !g_registry.system_initialized) {
        return -1;
    }

    /* Find the module */
    int module_index = -1;
    for (int i = 0; i < g_registry.module_count; i++) {
        if (strcmp(g_registry.modules[i]->name, name) == 0) {
            module_index = i;
            break;
        }
    }

    if (module_index < 0) {
        fprintf(stderr, "Error: Module '%s' not found for unregistration\n", name);
        return -2;
    }

    physics_module_t *module = g_registry.modules[module_index];

    /* Cleanup module if it's initialized */
    if (module->initialized) {
        if (module->cleanup) {
            module->cleanup();
        }
        module->initialized = false;
        module->active = false;
    }

    /* Remove module from registry by shifting array */
    for (int i = module_index; i < g_registry.module_count - 1; i++) {
        g_registry.modules[i] = g_registry.modules[i + 1];
    }
    g_registry.modules[g_registry.module_count - 1] = NULL;
    g_registry.module_count--;

    return 0;
}

/**
 * @brief Get list of all registered modules
 *
 * @param modules Array to store module pointers (caller-allocated)
 * @param max_count Maximum number of modules to return
 * @return Number of modules returned, negative on error
 */
int get_registered_modules(physics_module_t **modules, int max_count) {
    if (!modules || max_count <= 0 || !g_registry.system_initialized) {
        return -1;
    }

    int count = (g_registry.module_count < max_count) ? g_registry.module_count : max_count;

    for (int i = 0; i < count; i++) {
        modules[i] = g_registry.modules[i];
    }

    return count;
}

/**
 * @brief Initialize all registered modules
 *
 * @return 0 on success, negative on error
 */
int initialize_all_modules(void) {
    if (!g_registry.system_initialized) {
        return -1;
    }

    int failed_count = 0;

    for (int i = 0; i < g_registry.module_count; i++) {
        physics_module_t *module = g_registry.modules[i];

        if (module->initialized) {
            continue; /* Already initialized */
        }

        if (module->init) {
            int result = module->init();
            if (result == 0) {
                module->initialized = true;
                module->active = true;
            } else {
                fprintf(stderr, "Warning: Failed to initialize module '%s' (error %d)\n",
                        module->name, result);
                failed_count++;
            }
        } else {
            fprintf(stderr, "Warning: Module '%s' has no init function\n", module->name);
            failed_count++;
        }
    }

    return (failed_count > 0) ? -failed_count : 0;
}

/**
 * @brief Execute a specific physics phase across all capable modules
 *
 * @param phase Physics phase to execute
 * @param context Execution context for the phase
 * @return 0 on success, negative on error
 */
int execute_physics_phase(physics_execution_phase_t phase,
                         physics_execution_context_t *context) {
    if (!context || !g_registry.system_initialized) {
        return -1;
    }

    int executed_count = 0;
    int failed_count = 0;

    for (int i = 0; i < g_registry.module_count; i++) {
        physics_module_t *module = g_registry.modules[i];

        if (!module->initialized || !module->active) {
            continue; /* Skip uninitialized or inactive modules */
        }

        if (module->execute) {
            int result = module->execute(phase, context);
            if (result == 0) {
                executed_count++;
            } else {
                fprintf(stderr, "Warning: Module '%s' failed phase execution (error %d)\n",
                        module->name, result);
                failed_count++;
            }
        }
    }

    return (failed_count > 0) ? -failed_count : executed_count;
}

/**
 * @brief Cleanup all registered modules
 *
 * @return 0 on success, negative on error
 */
int cleanup_all_modules(void) {
    if (!g_registry.system_initialized) {
        return -1;
    }

    int failed_count = 0;

    for (int i = 0; i < g_registry.module_count; i++) {
        physics_module_t *module = g_registry.modules[i];

        if (!module->initialized) {
            continue; /* Skip uninitialized modules */
        }

        if (module->cleanup) {
            int result = module->cleanup();
            if (result != 0) {
                fprintf(stderr, "Warning: Failed to cleanup module '%s' (error %d)\n",
                        module->name, result);
                failed_count++;
            }
        }

        module->initialized = false;
        module->active = false;
    }

    return (failed_count > 0) ? -failed_count : 0;
}

/**
 * @brief Check if a module has specific capabilities
 *
 * @param module Module to check
 * @param capabilities Capability flags to test
 * @return true if module has all specified capabilities
 */
bool module_has_capabilities(const physics_module_t *module, uint32_t capabilities) {
    if (!module) {
        return false;
    }

    return (module->capabilities & capabilities) == capabilities;
}

/**
 * @brief Get modules that provide specific capabilities
 *
 * @param capabilities Required capability flags
 * @param modules Array to store matching module pointers
 * @param max_count Maximum number of modules to return
 * @return Number of matching modules, negative on error
 */
int get_modules_with_capabilities(uint32_t capabilities,
                                  physics_module_t **modules, int max_count) {
    if (!modules || max_count <= 0 || !g_registry.system_initialized) {
        return -1;
    }

    int found_count = 0;

    for (int i = 0; i < g_registry.module_count && found_count < max_count; i++) {
        physics_module_t *module = g_registry.modules[i];

        if (module_has_capabilities(module, capabilities)) {
            modules[found_count] = module;
            found_count++;
        }
    }

    return found_count;
}

/**
 * @brief Validate module dependencies (basic implementation)
 *
 * @param module Module to validate
 * @return 0 if dependencies satisfied, negative on error
 */
int validate_module_dependencies(const physics_module_t *module) {
    if (!module || !g_registry.system_initialized) {
        return -1;
    }

    /* If no dependencies specified, validation passes */
    if (strlen(module->dependencies) == 0) {
        return 0;
    }

    /* Basic dependency validation: check if named modules are registered */
    char deps_copy[MAX_DEPENDENCY_LEN];
    strncpy(deps_copy, module->dependencies, MAX_DEPENDENCY_LEN - 1);
    deps_copy[MAX_DEPENDENCY_LEN - 1] = '\0';

    char *token = strtok(deps_copy, ",");
    while (token != NULL) {
        /* Remove leading/trailing whitespace (simple implementation) */
        while (*token == ' ') token++;
        char *end = token + strlen(token) - 1;
        while (end > token && *end == ' ') *end-- = '\0';

        /* Check if dependency is registered */
        if (find_physics_module(token) == NULL) {
            fprintf(stderr, "Error: Module '%s' depends on unregistered module '%s'\n",
                    module->name, token);
            return -2;
        }

        token = strtok(NULL, ",");
    }

    return 0;
}

/**
 * @brief Print information about all registered modules
 */
void print_registered_modules(void) {
    if (!g_registry.system_initialized) {
        printf("Physics module system not initialized\n");
        return;
    }

    printf("Registered Physics Modules (%d/%d):\n",
           g_registry.module_count, MAX_REGISTERED_MODULES);

    if (g_registry.module_count == 0) {
        printf("  No modules registered\n");
        return;
    }

    for (int i = 0; i < g_registry.module_count; i++) {
        physics_module_t *module = g_registry.modules[i];

        printf("  [%d] %s v%s\n", i, module->name, module->version);
        printf("      Description: %s\n", module->description);
        printf("      Capabilities: 0x%08X\n", module->capabilities);
        printf("      Dependencies: %s\n",
               strlen(module->dependencies) > 0 ? module->dependencies : "none");
        printf("      Priority: %d\n", module->priority);
        printf("      Status: %s, %s\n",
               module->initialized ? "initialized" : "uninitialized",
               module->active ? "active" : "inactive");
    }
}

/**
 * @brief Get string representation of capabilities
 *
 * @param capabilities Capability flags to convert
 * @param buffer Buffer to store string (caller-allocated)
 * @param buffer_size Size of buffer
 * @return Number of characters written, negative on error
 */
int capabilities_to_string(uint32_t capabilities, char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) {
        return -1;
    }

    if (capabilities == CAPABILITY_NONE) {
        return snprintf(buffer, buffer_size, "none");
    }

    char *start = buffer;
    size_t remaining = buffer_size;
    int written = 0;
    bool first = true;

    struct {
        uint32_t flag;
        const char *name;
    } capability_names[] = {
        {CAPABILITY_INFALL, "infall"},
        {CAPABILITY_COOLING, "cooling"},
        {CAPABILITY_STAR_FORMATION, "star_formation"},
        {CAPABILITY_FEEDBACK, "feedback"},
        {CAPABILITY_MERGERS, "mergers"},
        {CAPABILITY_DISK_INSTABILITY, "disk_instability"},
        {CAPABILITY_REINCORPORATION, "reincorporation"},
        {CAPABILITY_AGN, "agn"}
    };

    for (size_t i = 0; i < sizeof(capability_names) / sizeof(capability_names[0]); i++) {
        if (capabilities & capability_names[i].flag) {
            int len = snprintf(start, remaining, "%s%s",
                             first ? "" : ",", capability_names[i].name);
            if (len >= (int)remaining) {
                return -2; /* Buffer too small */
            }
            start += len;
            remaining -= len;
            written += len;
            first = false;
        }
    }

    return written;
}

/**
 * @brief Get string representation of execution phase
 *
 * @param phase Execution phase to convert
 * @return String representation (static storage)
 */
const char *phase_to_string(physics_execution_phase_t phase) {
    switch (phase) {
        case PHYSICS_PHASE_INIT:         return "init";
        case PHYSICS_PHASE_INFALL:       return "infall";
        case PHYSICS_PHASE_COOLING:      return "cooling";
        case PHYSICS_PHASE_STAR_FORMATION: return "star_formation";
        case PHYSICS_PHASE_MERGERS:      return "mergers";
        case PHYSICS_PHASE_MISC:         return "misc";
        case PHYSICS_PHASE_CLEANUP:      return "cleanup";
        default:                         return "unknown";
    }
}