# SAGE Physics Module Developer Guide

## Overview

This guide documents the SAGE physics module system, enabling developers to create and integrate new physical models into the SAGE galaxy evolution framework. The module interface was implemented in Task 2A.1 as part of establishing the physics-agnostic core architecture.

**Current Status**: Task 2A.1 Complete - Basic interface implemented
**Next Phase**: Task 2A.2 - Integration with core evolution pipeline

## Module Interface Implementation

### 1. Core Module Structure

The SAGE physics module interface is defined in `src/core/physics_module.h` and centers around the `physics_module_t` structure:

```c
typedef struct {
    char name[64];                    // Unique module name
    char version[16];                 // Module version string
    char description[128];            // Brief module description

    // Module lifecycle functions
    int (*init)(void);                // Initialize module
    int (*execute)(physics_execution_phase_t phase,
                   physics_execution_context_t *context);
    int (*cleanup)(void);             // Cleanup module resources

    // Module capabilities and dependencies
    uint32_t capabilities;            // Capability flags
    char dependencies[256];           // Comma-separated dependency list
    int priority;                     // Execution priority

    // Module runtime state
    bool initialized;                 // Module initialization status
    bool active;                     // Module active status
    void *private_data;              // Module-specific data
} physics_module_t;
```

### 2. Execution Phases

Physics modules execute during standardized phases that align with the evolution pipeline:

- `PHYSICS_PHASE_INIT` - Module initialization
- `PHYSICS_PHASE_INFALL` - Cosmological gas infall
- `PHYSICS_PHASE_COOLING` - Gas cooling processes
- `PHYSICS_PHASE_STAR_FORMATION` - Star formation and feedback
- `PHYSICS_PHASE_MERGERS` - Galaxy mergers and interactions
- `PHYSICS_PHASE_MISC` - Miscellaneous processes
- `PHYSICS_PHASE_CLEANUP` - Module cleanup

### 3. Capability System

Modules declare their capabilities using flags:

- `CAPABILITY_INFALL` - Provides cosmological infall
- `CAPABILITY_COOLING` - Provides gas cooling
- `CAPABILITY_STAR_FORMATION` - Provides star formation
- `CAPABILITY_FEEDBACK` - Provides stellar feedback
- `CAPABILITY_MERGERS` - Provides merger handling
- `CAPABILITY_DISK_INSTABILITY` - Provides disk instability
- `CAPABILITY_REINCORPORATION` - Provides gas reincorporation
- `CAPABILITY_AGN` - Provides AGN physics

### 4. Module Registration

Modules are registered with the physics module system using these functions:

```c
// Initialize the module system
int initialize_physics_module_system(void);

// Register a module
int register_physics_module(physics_module_t *module);

// Find a registered module
physics_module_t *find_physics_module(const char *name);

// Initialize all registered modules
int initialize_all_modules(void);

// Execute a physics phase
int execute_physics_phase(physics_execution_phase_t phase,
                         physics_execution_context_t *context);
```

### 5. Example Module Template

```c
// Example basic physics module
static physics_module_t my_physics_module = {
    .name = "my_physics",
    .version = "1.0.0",
    .description = "Example physics module",
    .init = my_physics_init,
    .execute = my_physics_execute,
    .cleanup = my_physics_cleanup,
    .capabilities = CAPABILITY_STAR_FORMATION | CAPABILITY_FEEDBACK,
    .dependencies = "",
    .priority = 10
};

static int my_physics_init(void) {
    // Initialize module-specific data
    return 0;
}

static int my_physics_execute(physics_execution_phase_t phase,
                             physics_execution_context_t *context) {
    // Execute physics based on phase
    switch (phase) {
        case PHYSICS_PHASE_STAR_FORMATION:
            // Implement star formation
            break;
        // Handle other phases...
    }
    return 0;
}

static int my_physics_cleanup(void) {
    // Cleanup module resources
    return 0;
}
```

### 6. Module Dependencies and Testing

Dependencies are declared as comma-separated module names:
```c
.dependencies = "cooling_module,infall_module"
```

The system validates dependencies at registration and can order execution based on dependencies.

**Testing**: All modules should be tested using the SAGE testing framework. See `tests/test_physics_module.c` for examples of comprehensive module testing.

## Current Implementation Status

**✅ Task 2A.1 Complete**: Basic interface implemented
- Module structure and lifecycle management
- Registration and lookup system
- Capability declarations and dependency validation
- Comprehensive unit test coverage

**🚧 Next: Task 2A.2**: Integration with core evolution pipeline
- Replace direct physics calls in `evolution.c`
- Enable conditional physics execution
- Achieve physics-agnostic core operation

## Future Development (Tasks 2A.3-2A.6)

### Planned Enhancements
- **Task 2A.3**: Physics-free mode implementation
- **Task 2A.4**: Legacy physics module wrapping
- **Task 2A.5**: Enhanced dependency framework
- **Task 2A.6**: Complete developer documentation

### Advanced Features (Post-2A)
- Runtime module configuration via YAML
- Inter-module communication patterns
- Property system integration
- Module-aware memory management

## Related Documentation

- [SAGE Master Plan](../log/sage-master-plan.md) - Complete architectural transformation roadmap
- [Architecture Vision](../log/sage-architecture-vision.md) - Long-term architectural principles
- [Current Architecture](../log/architecture.md) - Current codebase structure
- [Directory Structure](directory-structure.md) - Code organization guide

## Contributing

Once the physics module system is implemented in Phase 2A, this guide will provide the definitive reference for:

- Physics researchers wanting to implement new models
- Developers extending SAGE capabilities
- Contributors modernizing existing physics implementations
- Users configuring module combinations

For current development questions, refer to the [project documentation index](quick-reference.md) and the development guidelines in [CLAUDE.md](../CLAUDE.md).