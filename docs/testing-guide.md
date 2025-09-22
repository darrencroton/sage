# SAGE Testing Framework Guide

This document describes the SAGE testing framework, including how to write, run, and maintain unit tests for the SAGE galaxy evolution model.

## Overview

SAGE uses a modern testing framework built on:
- **CTest**: CMake's testing framework for test discovery and execution
- **Custom test utilities**: Standardized macros and reporting in `tests/test_runner.h`
- **GitHub Actions**: Automated continuous integration testing
- **Memory validation**: Integration with SAGE's memory management system

## Running Tests

### Local Testing

```bash
# Build SAGE with tests
mkdir build && cd build
cmake .. && make -j$(nproc)

# Run all tests
ctest --output-on-failure

# Run tests in parallel
ctest --parallel 4

# Run specific test
ctest -R test_numeric

# Verbose output
ctest --verbose

# Run tests and continue on failure
ctest --continue-on-failure
```

### Individual Test Execution

```bash
# Run individual test directly
cd build
./tests/test_numeric

# Run with different working directory
cd build/tests
./test_numeric
```

## Writing Unit Tests

### Basic Test Structure

Each test file should follow this pattern:

```c
#include "test_runner.h"
#include "module_to_test.h"

// Individual test functions
void test_functionality() {
    // Test implementation using ASSERT macros
    ASSERT_EQ(function_call(), expected_value);
    ASSERT_TRUE(condition);
}

// Main test runner
int main() {
    TEST_SUITE_START();

    RUN_TEST(test_functionality);
    // Add more tests...

    TEST_SUITE_END();
}
```

### Available Test Macros

The `test_runner.h` header provides these assertion macros:

#### Basic Assertions
- `ASSERT_EQ(actual, expected)` - Test equality
- `ASSERT_NEAR(actual, expected, tolerance)` - Test floating-point near equality
- `ASSERT_TRUE(condition)` - Test boolean true
- `ASSERT_FALSE(condition)` - Test boolean false
- `ASSERT_STR_EQ(actual, expected)` - Test string equality

#### Test Management
- `TEST_SUITE_START()` - Initialize test suite
- `TEST_SUITE_END()` - Finalize and report results
- `RUN_TEST(test_function)` - Execute individual test
- `TEST_START(name)` - Manual test start reporting
- `TEST_PASS(name)` - Manual test pass reporting
- `TEST_FAIL(name, message)` - Manual test failure

### Example: Testing Numeric Functions

```c
#include "test_runner.h"
#include "numeric.h"

void test_safe_division() {
    // Test normal division
    ASSERT_NEAR(safe_div(10.0, 2.0, 0.0), 5.0, 1e-10);

    // Test division by zero protection
    ASSERT_EQ(safe_div(10.0, 0.0, -1.0), -1.0);

    // Test with very small denominators (considered zero)
    ASSERT_EQ(safe_div(1.0, 1e-20, 42.0), 42.0);
}

void test_value_comparisons() {
    // Test floating-point equality
    ASSERT_TRUE(is_equal(1.0, 1.0));
    ASSERT_FALSE(is_equal(1.0, 2.0));

    // Test zero checking
    ASSERT_TRUE(is_zero(0.0));
    ASSERT_TRUE(is_zero(1e-15));  // Below EPSILON_SMALL
    ASSERT_FALSE(is_zero(1e-5)); // Above EPSILON_SMALL
}

int main() {
    TEST_SUITE_START();

    RUN_TEST(test_safe_division);
    RUN_TEST(test_value_comparisons);

    TEST_SUITE_END();
}
```

## Adding New Tests

### 1. Create Test File

Create a new `.c` file in the `tests/` directory:

```bash
# Create test file
touch tests/test_mymodule.c
```

### 2. Implement Test

Use the standard test structure and include necessary headers:

```c
#include "test_runner.h"
#include "mymodule.h"  // Module being tested

void test_mymodule_function() {
    // Test implementation
}

int main() {
    TEST_SUITE_START();
    RUN_TEST(test_mymodule_function);
    TEST_SUITE_END();
}
```

### 3. Update CMakeLists.txt

The test will be automatically detected if the file exists. The `tests/CMakeLists.txt` includes conditional compilation:

```cmake
# Tests are automatically detected by filename pattern
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_mymodule.c")
    add_sage_test(test_mymodule test_mymodule.c ../src/path/to/module.c)
    message(STATUS "MyModule tests enabled")
endif()
```

For custom test configurations, you can manually add:

```cmake
add_sage_test(test_mymodule test_mymodule.c
    ../src/path/to/module.c
    ../src/other/dependency.c)
```

## Test Categories

### Unit Tests
- **Purpose**: Test individual functions in isolation
- **Location**: `tests/test_*.c`
- **Scope**: Single module or utility functions
- **Example**: `test_numeric.c`, `test_memory.c`

### Integration Tests
- **Purpose**: Test interaction between components
- **Location**: `tests/integration_*.c` (planned)
- **Scope**: Multiple modules working together
- **Example**: Configuration + I/O integration

### System Tests
- **Purpose**: End-to-end functionality testing
- **Location**: `tests/system_*.c` (planned)
- **Scope**: Complete workflows
- **Example**: Full simulation pipeline testing

## Memory Testing

SAGE's comprehensive memory management system integrates with tests:

### Memory Leak Detection

```c
#include "test_runner.h"
#include "memory.h"

int main() {
    // Initialize memory system for tests
    initialize_error_handling(2, stdout);
    init_memory_system(1024);

    TEST_SUITE_START();

    // Run tests...
    RUN_TEST(my_test);

    // Check for memory leaks
    printf("\nMemory leak check:\n");
    check_memory_leaks();

    TEST_SUITE_END();
}
```

### Memory Category Testing

Tests can verify memory allocation patterns:

```c
void test_memory_usage() {
    size_t initial_memory = get_memory_usage();

    // Perform operations that allocate memory
    void *ptr = mymalloc(1000);

    // Verify memory increase
    ASSERT_TRUE(get_memory_usage() > initial_memory);

    // Clean up
    myfree(ptr);

    // Verify memory return to initial state
    ASSERT_EQ(get_memory_usage(), initial_memory);
}
```

## Continuous Integration

### GitHub Actions Workflow

The CI system automatically:

1. **Multi-platform testing**: Ubuntu and macOS
2. **Multi-configuration**: Debug and Release builds
3. **Dependency variations**: With and without optional dependencies
4. **Code quality**: Formatting and warning checks
5. **Test execution**: All CTest tests with failure reporting

### Test Requirements for CI

Tests should be:
- **Self-contained**: No external file dependencies
- **Fast**: Complete within reasonable time limits
- **Deterministic**: Same results on repeated runs
- **Cross-platform**: Work on Ubuntu and macOS

### Debugging CI Failures

```bash
# Reproduce CI environment locally
docker run -it ubuntu:latest
apt-get update && apt-get install -y libyaml-dev cmake build-essential

# Build and test as CI does
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

## Best Practices

### Test Design
1. **One concept per test function**: Focus on testing single functionality
2. **Descriptive names**: `test_safe_div_by_zero()` not `test_division()`
3. **Clear assertions**: Use appropriate ASSERT macros for readability
4. **Edge cases**: Test boundary conditions and error cases

### Error Handling
1. **Test error conditions**: Verify proper error handling
2. **Expected failures**: Test that invalid inputs fail appropriately
3. **Resource cleanup**: Ensure tests clean up after themselves

### Performance
1. **Fast execution**: Tests should complete quickly
2. **Minimal setup**: Avoid unnecessary initialization
3. **Parallel-safe**: Tests should not interfere with each other

### Maintenance
1. **Update with code changes**: Keep tests current with implementation
2. **Document complex tests**: Explain non-obvious test logic
3. **Regular review**: Periodically review test coverage and effectiveness

## Troubleshooting

### Common Issues

#### Tests Don't Build
- **Check includes**: Ensure all necessary headers are included
- **Verify CMakeLists.txt**: Confirm test is properly configured
- **Dependency order**: Ensure source files are listed in correct order

#### Tests Fail Unexpectedly
- **Floating-point precision**: Use `ASSERT_NEAR` for floating-point comparisons
- **Epsilon values**: Understand SAGE's epsilon constants (EPSILON_SMALL = 1e-10)
- **Platform differences**: Test on target platforms

#### Memory Issues
- **Initialize memory system**: Call `init_memory_system()` before tests
- **Check for leaks**: Always call `check_memory_leaks()` after tests
- **Clean up**: Ensure all allocations are freed

### Debug Commands

```bash
# Run tests with debugger
cd build
lldb ./tests/test_numeric
# In lldb: run, bt, print <variable>

# Run with memory checking (if available)
valgrind --leak-check=full ./tests/test_numeric

# Verbose CMake output
cd build
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make test_numeric VERBOSE=1
```

## Test Template

Use this template for new test files:

```c
/**
 * @file    test_template.c
 * @brief   Unit tests for [module name]
 *
 * This file contains unit tests for [description of what is being tested].
 * Tests verify [key functionality being validated].
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "test_runner.h"
#include "module_to_test.h"

/**
 * @brief Test [specific functionality]
 */
void test_functionality_name() {
    // Arrange - set up test data

    // Act - perform the operation being tested

    // Assert - verify the results
    ASSERT_EQ(result, expected);
}

/**
 * @brief Test error conditions
 */
void test_error_conditions() {
    // Test invalid inputs, edge cases, etc.
}

/**
 * @brief Main test runner
 */
int main() {
    // Initialize systems if needed
    initialize_error_handling(2, stdout);
    init_memory_system(1024);

    TEST_SUITE_START();

    RUN_TEST(test_functionality_name);
    RUN_TEST(test_error_conditions);

    // Check for memory leaks
    printf("\nMemory leak check:\n");
    check_memory_leaks();

    TEST_SUITE_END();
}
```

## Future Enhancements

Planned improvements to the testing framework:

1. **Test coverage reporting**: Integration with gcov/lcov
2. **Performance benchmarking**: Automated performance regression detection
3. **Integration test suite**: Multi-component testing framework
4. **Mock framework**: For testing components with external dependencies
5. **Fuzzing support**: Automated input generation for robustness testing

## Contributing

When adding new functionality to SAGE:

1. **Write tests first**: Consider test-driven development
2. **Maintain test coverage**: Ensure new code is tested
3. **Run full test suite**: Verify no regressions before committing
4. **Update documentation**: Keep this guide current with changes

For questions or issues with the testing framework, refer to the project documentation or create an issue in the repository.