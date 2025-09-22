/**
 * @file    test_runner.h
 * @brief   Common test utilities and macros for SAGE unit tests
 *
 * This header provides standardized testing utilities for the SAGE
 * testing framework, including assertion macros, test reporting,
 * and memory validation utilities.
 */

#ifndef TEST_RUNNER_H
#define TEST_RUNNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test reporting macros
#define TEST_PASS(test_name) \
    printf("✓ %s passed\n", test_name)

#define TEST_FAIL(test_name, message) \
    do { \
        printf("✗ %s failed: %s\n", test_name, message); \
        exit(1); \
    } while(0)

#define TEST_START(test_name) \
    printf("Running %s...\n", test_name)

// Enhanced assertion macros
#define ASSERT_EQ(actual, expected) \
    do { \
        if ((actual) != (expected)) { \
            fprintf(stderr, "Assertion failed: %s == %s (%g != %g) at %s:%d\n", \
                    #actual, #expected, (double)(actual), (double)(expected), \
                    __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define ASSERT_NEAR(actual, expected, tolerance) \
    do { \
        double diff = fabs((double)(actual) - (double)(expected)); \
        if (diff > (tolerance)) { \
            fprintf(stderr, "Assertion failed: %s ≈ %s (|%g - %g| = %g > %g) at %s:%d\n", \
                    #actual, #expected, (double)(actual), (double)(expected), \
                    diff, (double)(tolerance), __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "Assertion failed: %s at %s:%d\n", \
                    #condition, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            fprintf(stderr, "Assertion failed: !(%s) at %s:%d\n", \
                    #condition, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

#define ASSERT_STR_EQ(actual, expected) \
    do { \
        if (strcmp((actual), (expected)) != 0) { \
            fprintf(stderr, "Assertion failed: %s == %s (\"%s\" != \"%s\") at %s:%d\n", \
                    #actual, #expected, (actual), (expected), __FILE__, __LINE__); \
            exit(1); \
        } \
    } while(0)

// Test suite management
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} test_suite_t;

static test_suite_t g_test_suite = {0, 0, 0};

#define TEST_SUITE_START() \
    do { \
        g_test_suite.total_tests = 0; \
        g_test_suite.passed_tests = 0; \
        g_test_suite.failed_tests = 0; \
        printf("Starting test suite...\n\n"); \
    } while(0)

#define TEST_SUITE_END() \
    do { \
        printf("\n=== Test Suite Results ===\n"); \
        printf("Total tests:  %d\n", g_test_suite.total_tests); \
        printf("Passed tests: %d\n", g_test_suite.passed_tests); \
        printf("Failed tests: %d\n", g_test_suite.failed_tests); \
        if (g_test_suite.failed_tests == 0) { \
            printf("✓ All tests passed!\n"); \
            return 0; \
        } else { \
            printf("✗ %d test(s) failed\n", g_test_suite.failed_tests); \
            return 1; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        g_test_suite.total_tests++; \
        printf("Running " #test_func "...\n"); \
        test_func(); \
        g_test_suite.passed_tests++; \
        printf("✓ " #test_func " passed\n\n"); \
    } while(0)

#endif // TEST_RUNNER_H