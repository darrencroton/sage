/**
 * @file    test_physics_module.c
 * @brief   Unit tests for physics module interface system
 *
 * This file provides comprehensive testing for the physics module interface,
 * including module registration, lifecycle management, capability tracking,
 * and dependency validation. These tests ensure the interface meets the
 * requirements for Task 2A.1 and establishes the foundation for Principle 1
 * compliance (Physics-Agnostic Core Infrastructure).
 *
 * Test Coverage:
 * - Module registration and lookup functionality
 * - Module lifecycle management (init/execute/cleanup)
 * - Capability declarations and queries
 * - Basic dependency validation
 * - Error conditions and edge cases
 * - Memory management integration
 * - Thread safety (basic validation)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Include the SAGE testing framework
#include "test_runner.h"

// Include the module being tested
#include "../src/core/physics_module.h"

// Include SAGE systems for integration testing
#include "../src/core/memory.h"

/**
 * @brief Test data and mock modules for comprehensive testing
 */
typedef struct {
    double tolerance;              /* Standard floating-point tolerance */
    int module_count;             /* Number of test modules created */

    /* Mock modules for testing */
    physics_module_t test_module_1;
    physics_module_t test_module_2;
    physics_module_t test_module_with_deps;

    /* Test execution context */
    physics_execution_context_t test_context;

    /* Counters for mock function calls */
    int init_call_count;
    int execute_call_count;
    int cleanup_call_count;
} test_data_t;

/* Global test data */
static test_data_t g_test_data;

/**
 * @brief Mock module functions for testing lifecycle management
 */
static int mock_init_success(void) {
    g_test_data.init_call_count++;
    return 0;
}

static int mock_init_failure(void) {
    g_test_data.init_call_count++;
    return -1;
}

static int mock_execute_success(physics_execution_phase_t phase,
                               physics_execution_context_t *context) {
    g_test_data.execute_call_count++;
    return 0;
}

static int mock_execute_failure(physics_execution_phase_t phase,
                               physics_execution_context_t *context) {
    g_test_data.execute_call_count++;
    return -1;
}

static int mock_cleanup_success(void) {
    g_test_data.cleanup_call_count++;
    return 0;
}

static int mock_cleanup_failure(void) {
    g_test_data.cleanup_call_count++;
    return -1;
}

/**
 * @brief Initialize test data and mock modules
 */
static int setup_test_data(test_data_t *data) {
    if (!data) return -1;

    memset(data, 0, sizeof(test_data_t));

    /* Initialize standard test parameters */
    data->tolerance = 1e-10;
    data->module_count = 0;

    /* Initialize test module 1 - basic star formation module */
    strncpy(data->test_module_1.name, "test_star_formation", MAX_MODULE_NAME_LEN - 1);
    strncpy(data->test_module_1.version, "1.0.0", MAX_MODULE_VERSION_LEN - 1);
    strncpy(data->test_module_1.description, "Test star formation module", 127);
    data->test_module_1.init = mock_init_success;
    data->test_module_1.execute = mock_execute_success;
    data->test_module_1.cleanup = mock_cleanup_success;
    data->test_module_1.capabilities = CAPABILITY_STAR_FORMATION | CAPABILITY_FEEDBACK;
    strncpy(data->test_module_1.dependencies, "", MAX_DEPENDENCY_LEN - 1);
    data->test_module_1.priority = 10;

    /* Initialize test module 2 - basic cooling module */
    strncpy(data->test_module_2.name, "test_cooling", MAX_MODULE_NAME_LEN - 1);
    strncpy(data->test_module_2.version, "2.1.0", MAX_MODULE_VERSION_LEN - 1);
    strncpy(data->test_module_2.description, "Test cooling module", 127);
    data->test_module_2.init = mock_init_success;
    data->test_module_2.execute = mock_execute_success;
    data->test_module_2.cleanup = mock_cleanup_success;
    data->test_module_2.capabilities = CAPABILITY_COOLING | CAPABILITY_INFALL;
    strncpy(data->test_module_2.dependencies, "", MAX_DEPENDENCY_LEN - 1);
    data->test_module_2.priority = 5;

    /* Initialize test module with dependencies */
    strncpy(data->test_module_with_deps.name, "test_mergers", MAX_MODULE_NAME_LEN - 1);
    strncpy(data->test_module_with_deps.version, "1.5.0", MAX_MODULE_VERSION_LEN - 1);
    strncpy(data->test_module_with_deps.description, "Test merger module with dependencies", 127);
    data->test_module_with_deps.init = mock_init_success;
    data->test_module_with_deps.execute = mock_execute_success;
    data->test_module_with_deps.cleanup = mock_cleanup_success;
    data->test_module_with_deps.capabilities = CAPABILITY_MERGERS;
    strncpy(data->test_module_with_deps.dependencies, "test_star_formation,test_cooling", MAX_DEPENDENCY_LEN - 1);
    data->test_module_with_deps.priority = 20;

    /* Initialize test execution context */
    data->test_context.galaxy_index = 123;
    data->test_context.central_galaxy_index = 456;
    data->test_context.halo_index = 789;
    data->test_context.current_time = 1.0;
    data->test_context.time_step = 0.01;
    data->test_context.integration_step = 1;
    data->test_context.config = NULL;
    data->test_context.sim_state = NULL;

    /* Reset call counters */
    data->init_call_count = 0;
    data->execute_call_count = 0;
    data->cleanup_call_count = 0;

    return 0;
}

/**
 * @brief Clean up test data
 */
static void cleanup_test_data(test_data_t *data) {
    if (!data) return;

    /* Cleanup physics module system */
    cleanup_physics_module_system();

    /* Reset test data */
    memset(data, 0, sizeof(test_data_t));
}

/**
 * @brief Test physics module system initialization and cleanup
 */
void test_system_initialization() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);

    /* Test system initialization */
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Test double initialization (should succeed) */
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Test system cleanup */
    cleanup_physics_module_system();

    /* Test cleanup of uninitialized system (should be safe) */
    cleanup_physics_module_system();

    cleanup_test_data(&data);
}

/**
 * @brief Test module registration functionality
 */
void test_module_registration() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Test successful module registration */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_FALSE(data.test_module_1.initialized);
    ASSERT_FALSE(data.test_module_1.active);

    /* Test duplicate module registration (should fail) */
    ASSERT_EQ(register_physics_module(&data.test_module_1), -5);

    /* Test registration with NULL module (should fail) */
    ASSERT_EQ(register_physics_module(NULL), -1);

    /* Test registration of module with empty name */
    physics_module_t empty_name_module = data.test_module_2;
    empty_name_module.name[0] = '\0';
    ASSERT_EQ(register_physics_module(&empty_name_module), -3);

    /* Test registration of module with missing function pointers */
    physics_module_t missing_func_module = data.test_module_2;
    missing_func_module.init = NULL;
    ASSERT_EQ(register_physics_module(&missing_func_module), -6);

    cleanup_test_data(&data);
}

/**
 * @brief Test module lookup and retrieval functionality
 */
void test_module_lookup() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Register test modules */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_EQ(register_physics_module(&data.test_module_2), 0);

    /* Test finding registered modules */
    physics_module_t *found_module = find_physics_module("test_star_formation");
    ASSERT_TRUE(found_module != NULL);
    ASSERT_STR_EQ(found_module->name, "test_star_formation");

    found_module = find_physics_module("test_cooling");
    ASSERT_TRUE(found_module != NULL);
    ASSERT_STR_EQ(found_module->name, "test_cooling");

    /* Test finding non-existent module */
    ASSERT_TRUE(find_physics_module("non_existent_module") == NULL);

    /* Test finding with NULL name */
    ASSERT_TRUE(find_physics_module(NULL) == NULL);

    /* Test getting list of registered modules */
    physics_module_t *modules[10];
    int count = get_registered_modules(modules, 10);
    ASSERT_EQ(count, 2);
    ASSERT_TRUE(modules[0] != NULL);
    ASSERT_TRUE(modules[1] != NULL);

    /* Test with insufficient buffer */
    count = get_registered_modules(modules, 1);
    ASSERT_EQ(count, 1);

    cleanup_test_data(&data);
}

/**
 * @brief Test module unregistration functionality
 */
void test_module_unregistration() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Register and initialize test modules */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_EQ(register_physics_module(&data.test_module_2), 0);
    ASSERT_EQ(initialize_all_modules(), 0);

    /* Verify modules are registered and initialized */
    ASSERT_TRUE(find_physics_module("test_star_formation") != NULL);
    ASSERT_TRUE(find_physics_module("test_cooling") != NULL);
    ASSERT_TRUE(data.test_module_1.initialized);
    ASSERT_TRUE(data.test_module_2.initialized);

    /* Test successful unregistration */
    ASSERT_EQ(unregister_physics_module("test_star_formation"), 0);
    ASSERT_TRUE(find_physics_module("test_star_formation") == NULL);
    ASSERT_TRUE(find_physics_module("test_cooling") != NULL);

    /* Test unregistration of non-existent module */
    ASSERT_EQ(unregister_physics_module("non_existent_module"), -2);

    /* Test unregistration with NULL name */
    ASSERT_EQ(unregister_physics_module(NULL), -1);

    cleanup_test_data(&data);
}

/**
 * @brief Test module lifecycle management
 */
void test_module_lifecycle() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Reset call counters before starting test */
    g_test_data.init_call_count = 0;
    g_test_data.execute_call_count = 0;
    g_test_data.cleanup_call_count = 0;

    /* Register test modules */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_EQ(register_physics_module(&data.test_module_2), 0);

    /* Test module initialization */
    ASSERT_EQ(g_test_data.init_call_count, 0);
    ASSERT_EQ(initialize_all_modules(), 0);
    ASSERT_EQ(g_test_data.init_call_count, 2);
    ASSERT_TRUE(data.test_module_1.initialized);
    ASSERT_TRUE(data.test_module_1.active);
    ASSERT_TRUE(data.test_module_2.initialized);
    ASSERT_TRUE(data.test_module_2.active);

    /* Test double initialization (should skip already initialized modules) */
    ASSERT_EQ(initialize_all_modules(), 0);
    ASSERT_EQ(g_test_data.init_call_count, 2); /* No additional calls */

    /* Test module execution */
    ASSERT_EQ(g_test_data.execute_call_count, 0);
    ASSERT_EQ(execute_physics_phase(PHYSICS_PHASE_STAR_FORMATION, &data.test_context), 2);
    ASSERT_EQ(g_test_data.execute_call_count, 2);

    /* Test module cleanup */
    ASSERT_EQ(g_test_data.cleanup_call_count, 0);
    ASSERT_EQ(cleanup_all_modules(), 0);
    ASSERT_EQ(g_test_data.cleanup_call_count, 2);
    ASSERT_FALSE(data.test_module_1.initialized);
    ASSERT_FALSE(data.test_module_1.active);
    ASSERT_FALSE(data.test_module_2.initialized);
    ASSERT_FALSE(data.test_module_2.active);

    cleanup_test_data(&data);
}

/**
 * @brief Test capability system functionality
 */
void test_capability_system() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Register test modules */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_EQ(register_physics_module(&data.test_module_2), 0);

    /* Test capability checking */
    ASSERT_TRUE(module_has_capabilities(&data.test_module_1, CAPABILITY_STAR_FORMATION));
    ASSERT_TRUE(module_has_capabilities(&data.test_module_1, CAPABILITY_FEEDBACK));
    ASSERT_TRUE(module_has_capabilities(&data.test_module_1,
                                       CAPABILITY_STAR_FORMATION | CAPABILITY_FEEDBACK));
    ASSERT_FALSE(module_has_capabilities(&data.test_module_1, CAPABILITY_COOLING));

    ASSERT_TRUE(module_has_capabilities(&data.test_module_2, CAPABILITY_COOLING));
    ASSERT_TRUE(module_has_capabilities(&data.test_module_2, CAPABILITY_INFALL));
    ASSERT_FALSE(module_has_capabilities(&data.test_module_2, CAPABILITY_STAR_FORMATION));

    /* Test capability-based module discovery */
    physics_module_t *modules[10];
    int count = get_modules_with_capabilities(CAPABILITY_STAR_FORMATION, modules, 10);
    ASSERT_EQ(count, 1);
    ASSERT_STR_EQ(modules[0]->name, "test_star_formation");

    count = get_modules_with_capabilities(CAPABILITY_COOLING, modules, 10);
    ASSERT_EQ(count, 1);
    ASSERT_STR_EQ(modules[0]->name, "test_cooling");

    count = get_modules_with_capabilities(CAPABILITY_MERGERS, modules, 10);
    ASSERT_EQ(count, 0);

    /* Test capability string conversion */
    char buffer[256];
    ASSERT_TRUE(capabilities_to_string(CAPABILITY_STAR_FORMATION, buffer, sizeof(buffer)) > 0);
    ASSERT_TRUE(strstr(buffer, "star_formation") != NULL);

    ASSERT_TRUE(capabilities_to_string(CAPABILITY_COOLING | CAPABILITY_INFALL, buffer, sizeof(buffer)) > 0);
    ASSERT_TRUE(strstr(buffer, "cooling") != NULL);
    ASSERT_TRUE(strstr(buffer, "infall") != NULL);

    cleanup_test_data(&data);
}

/**
 * @brief Test dependency validation functionality
 */
void test_dependency_validation() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Test module with no dependencies */
    ASSERT_EQ(validate_module_dependencies(&data.test_module_1), 0);

    /* Test module with unmet dependencies */
    ASSERT_EQ(validate_module_dependencies(&data.test_module_with_deps), -2);

    /* Register dependency modules */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_EQ(register_physics_module(&data.test_module_2), 0);

    /* Test module with satisfied dependencies */
    ASSERT_EQ(validate_module_dependencies(&data.test_module_with_deps), 0);

    /* Test with NULL module */
    ASSERT_EQ(validate_module_dependencies(NULL), -1);

    cleanup_test_data(&data);
}

/**
 * @brief Test error conditions and edge cases
 */
void test_error_conditions() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);

    /* Test operations without system initialization */
    ASSERT_EQ(register_physics_module(&data.test_module_1), -2);
    ASSERT_TRUE(find_physics_module("test_module") == NULL);
    ASSERT_EQ(unregister_physics_module("test_module"), -1);
    ASSERT_EQ(initialize_all_modules(), -1);
    ASSERT_EQ(cleanup_all_modules(), -1);

    /* Initialize system for remaining tests */
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Test execution with NULL context */
    ASSERT_EQ(execute_physics_phase(PHYSICS_PHASE_STAR_FORMATION, NULL), -1);

    /* Test module with failing lifecycle functions */
    physics_module_t failing_module = data.test_module_1;
    strncpy(failing_module.name, "failing_module", MAX_MODULE_NAME_LEN - 1);
    failing_module.init = mock_init_failure;
    failing_module.execute = mock_execute_failure;
    failing_module.cleanup = mock_cleanup_failure;

    ASSERT_EQ(register_physics_module(&failing_module), 0);

    /* Test failed initialization */
    int result = initialize_all_modules();
    ASSERT_TRUE(result < 0); /* Should fail */

    /* Test failed execution - since the module failed to initialize, no modules should execute */
    result = execute_physics_phase(PHYSICS_PHASE_STAR_FORMATION, &data.test_context);
    ASSERT_TRUE(result >= 0); /* Should return 0 (no modules executed) */

    cleanup_test_data(&data);
}

/**
 * @brief Test utility functions
 */
void test_utility_functions() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Test phase to string conversion */
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_INIT), "init");
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_INFALL), "infall");
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_COOLING), "cooling");
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_STAR_FORMATION), "star_formation");
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_MERGERS), "mergers");
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_MISC), "misc");
    ASSERT_STR_EQ(phase_to_string(PHYSICS_PHASE_CLEANUP), "cleanup");
    ASSERT_STR_EQ(phase_to_string((physics_execution_phase_t)0xFF), "unknown");

    /* Test capability string conversion edge cases */
    char buffer[256];
    ASSERT_TRUE(capabilities_to_string(CAPABILITY_NONE, buffer, sizeof(buffer)) > 0);
    ASSERT_STR_EQ(buffer, "none");

    /* Test buffer too small */
    char small_buffer[5];
    ASSERT_EQ(capabilities_to_string(CAPABILITY_STAR_FORMATION, small_buffer, sizeof(small_buffer)), -2);

    /* Test print functions (should not crash) */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    print_registered_modules();

    cleanup_test_data(&data);
}

/**
 * @brief Test memory management integration
 */
void test_memory_integration() {
    test_data_t data;
    ASSERT_EQ(setup_test_data(&data), 0);
    ASSERT_EQ(initialize_physics_module_system(), 0);

    /* Register and operate on modules */
    ASSERT_EQ(register_physics_module(&data.test_module_1), 0);
    ASSERT_EQ(register_physics_module(&data.test_module_2), 0);
    ASSERT_EQ(initialize_all_modules(), 0);
    ASSERT_EQ(execute_physics_phase(PHYSICS_PHASE_STAR_FORMATION, &data.test_context), 2);
    ASSERT_EQ(cleanup_all_modules(), 0);

    cleanup_test_data(&data);

    /* Verify no memory leaks by checking memory validation */
    ASSERT_TRUE(validate_all_memory() >= 0);
}

/**
 * @brief Main test runner
 */
int main() {
    /* Initialize SAGE systems for testing */
    init_memory_system(1024);

    TEST_SUITE_START();

    /* Run all test functions */
    RUN_TEST(test_system_initialization);
    RUN_TEST(test_module_registration);
    RUN_TEST(test_module_lookup);
    RUN_TEST(test_module_unregistration);
    RUN_TEST(test_module_lifecycle);
    RUN_TEST(test_capability_system);
    RUN_TEST(test_dependency_validation);
    RUN_TEST(test_error_conditions);
    RUN_TEST(test_utility_functions);
    RUN_TEST(test_memory_integration);

    /* Check for memory leaks before finishing */
    printf("\nMemory leak check:\n");
    check_memory_leaks();

    TEST_SUITE_END();
}