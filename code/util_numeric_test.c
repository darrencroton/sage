/**
 * @file    util_numeric_test.c
 * @brief   Test program for the numeric utility functions
 *
 * This file contains a simple test program to validate the functionality
 * of the numeric utility functions. It can be compiled separately and
 * is not part of the main SAGE executable.
 *
 * Compile with: gcc -o util_numeric_test util_numeric_test.c util_numeric.c util_error.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util_numeric.h"
#include "constants.h"

/* Simple test macros */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("FAIL: %s\n", message); \
            failures++; \
        } else { \
            printf("PASS: %s\n", message); \
            passes++; \
        } \
    } while (0)

int main(void)
{
    int passes = 0, failures = 0;
    
    printf("Testing numeric utility functions\n");
    printf("=================================\n\n");

    /* Test is_zero */
    TEST_ASSERT(is_zero(0.0), "is_zero(0.0)");
    TEST_ASSERT(is_zero(EPSILON_SMALL * 0.5), "is_zero with value < EPSILON_SMALL");
    TEST_ASSERT(!is_zero(EPSILON_SMALL * 10.0), "is_zero with value > EPSILON_SMALL");
    
    /* Test is_equal */
    TEST_ASSERT(is_equal(1.0, 1.0), "is_equal with identical values");
    TEST_ASSERT(is_equal(1.0, 1.0 + EPSILON_MEDIUM * 0.5), "is_equal with small difference");
    TEST_ASSERT(!is_equal(1.0, 1.1), "is_equal with significant difference");
    
    /* Test is_greater */
    TEST_ASSERT(is_greater(10.0, 9.0), "is_greater with clearly greater value");
    TEST_ASSERT(!is_greater(10.0, 10.0), "is_greater with equal values");
    TEST_ASSERT(!is_greater(10.0, 10.0 + EPSILON_SMALL * 0.5), "is_greater with tiny difference");
    
    /* Test is_less */
    TEST_ASSERT(is_less(9.0, 10.0), "is_less with clearly smaller value");
    TEST_ASSERT(!is_less(10.0, 10.0), "is_less with equal values");
    TEST_ASSERT(!is_less(10.0 - EPSILON_SMALL * 0.5, 10.0), "is_less with tiny difference");
    
    /* Test is_within */
    TEST_ASSERT(is_within(5.0, 1.0, 10.0), "is_within for value in range");
    TEST_ASSERT(is_within(1.0, 1.0, 10.0), "is_within at lower bound");
    TEST_ASSERT(is_within(10.0, 1.0, 10.0), "is_within at upper bound");
    TEST_ASSERT(!is_within(0.5, 1.0, 10.0), "is_within for value below range");
    TEST_ASSERT(!is_within(10.5, 1.0, 10.0), "is_within for value above range");
    
    /* Test safe_div */
    TEST_ASSERT(fabs(safe_div(10.0, 2.0, -1.0) - 5.0) < EPSILON_SMALL, "safe_div with non-zero denominator");
    TEST_ASSERT(fabs(safe_div(10.0, 0.0, -1.0) - (-1.0)) < EPSILON_SMALL, "safe_div with zero denominator");
    
    /* Test clamp */
    TEST_ASSERT(fabs(clamp(5.0, 1.0, 10.0) - 5.0) < EPSILON_SMALL, "clamp with value in range");
    TEST_ASSERT(fabs(clamp(0.5, 1.0, 10.0) - 1.0) < EPSILON_SMALL, "clamp with value below range");
    TEST_ASSERT(fabs(clamp(15.0, 1.0, 10.0) - 10.0) < EPSILON_SMALL, "clamp with value above range");
    
    /* Test is_finite_value */
    TEST_ASSERT(is_finite_value(5.0), "is_finite_value with normal value");
    TEST_ASSERT(!is_finite_value(INFINITY), "is_finite_value with infinity");
    TEST_ASSERT(!is_finite_value(NAN), "is_finite_value with NaN");
    
    /* Test sign */
    TEST_ASSERT(sign(5.0) == 1, "sign of positive value");
    TEST_ASSERT(sign(-5.0) == -1, "sign of negative value");
    TEST_ASSERT(sign(0.0) == 0, "sign of zero");
    
    /* Report test results */
    printf("\nTest results: %d passes, %d failures\n", passes, failures);
    
    return failures > 0 ? 1 : 0;
}
