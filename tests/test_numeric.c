/**
 * @file    test_numeric.c
 * @brief   Unit tests for numeric utility functions
 *
 * This file contains unit tests for the numeric utility functions
 * in src/utils/numeric.c, establishing the standard testing pattern
 * for the SAGE testing framework.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "test_runner.h"
#include "numeric.h"

/**
 * @brief Test the safe division function
 */
void test_safe_div() {
    // Test normal division
    ASSERT_NEAR(safe_div(10.0, 2.0, 0.0), 5.0, 1e-10);
    ASSERT_NEAR(safe_div(1.0, 3.0, 0.0), 1.0/3.0, 1e-10);

    // Test division by zero (should return default value)
    ASSERT_EQ(safe_div(10.0, 0.0, -1.0), -1.0);
    ASSERT_EQ(safe_div(-5.0, 0.0, 99.0), 99.0);

    // Test with very small denominators (1e-20 < EPSILON_SMALL, so should return default)
    ASSERT_EQ(safe_div(1.0, 1e-20, -99.0), -99.0);

    // Test with denominators larger than EPSILON_SMALL
    ASSERT_NEAR(safe_div(1.0, 1e-9, 0.0), 1e9, 1e3);

    // Test edge cases
    ASSERT_EQ(safe_div(0.0, 5.0, -1.0), 0.0);
    ASSERT_EQ(safe_div(0.0, 0.0, 42.0), 42.0);
}

/**
 * @brief Test zero checking function
 */
void test_is_zero() {
    // Test zero values
    ASSERT_TRUE(is_zero(0.0));
    ASSERT_TRUE(is_zero(1e-20));  // Very small value (< EPSILON_SMALL = 1e-10)
    ASSERT_TRUE(is_zero(-1e-20)); // Very small negative value
    ASSERT_TRUE(is_zero(1e-11));  // Just below EPSILON_SMALL

    // Test non-zero values (> EPSILON_SMALL = 1e-10)
    ASSERT_FALSE(is_zero(1.0));
    ASSERT_FALSE(is_zero(-1.0));
    ASSERT_FALSE(is_zero(0.001));
    ASSERT_FALSE(is_zero(1e-9));   // Just above EPSILON_SMALL
}

/**
 * @brief Test equality checking function
 */
void test_is_equal() {
    // Test exact equality
    ASSERT_TRUE(is_equal(1.0, 1.0));
    ASSERT_TRUE(is_equal(0.0, 0.0));

    // Test near equality (within tolerance)
    ASSERT_TRUE(is_equal(1.0, 1.0 + 1e-15));
    ASSERT_TRUE(is_equal(1.0, 1.0 - 1e-15));

    // Test clear inequality
    ASSERT_FALSE(is_equal(1.0, 2.0));
    ASSERT_FALSE(is_equal(0.0, 1.0));
}

/**
 * @brief Test comparison functions
 */
void test_comparisons() {
    // Test greater than
    ASSERT_TRUE(is_greater(2.0, 1.0));
    ASSERT_FALSE(is_greater(1.0, 2.0));
    ASSERT_FALSE(is_greater(1.0, 1.0));

    // Test less than
    ASSERT_TRUE(is_less(1.0, 2.0));
    ASSERT_FALSE(is_less(2.0, 1.0));
    ASSERT_FALSE(is_less(1.0, 1.0));

    // Test greater or equal
    ASSERT_TRUE(is_greater_or_equal(2.0, 1.0));
    ASSERT_TRUE(is_greater_or_equal(1.0, 1.0));
    ASSERT_FALSE(is_greater_or_equal(1.0, 2.0));

    // Test less or equal
    ASSERT_TRUE(is_less_or_equal(1.0, 2.0));
    ASSERT_TRUE(is_less_or_equal(1.0, 1.0));
    ASSERT_FALSE(is_less_or_equal(2.0, 1.0));
}

/**
 * @brief Test utility functions
 */
void test_utilities() {
    // Test clamp function
    ASSERT_EQ(clamp(5.0, 0.0, 10.0), 5.0);  // Within range
    ASSERT_EQ(clamp(-1.0, 0.0, 10.0), 0.0); // Below minimum
    ASSERT_EQ(clamp(15.0, 0.0, 10.0), 10.0); // Above maximum

    // Test is_within function
    ASSERT_TRUE(is_within(5.0, 0.0, 10.0));
    ASSERT_TRUE(is_within(0.0, 0.0, 10.0));  // At minimum
    ASSERT_TRUE(is_within(10.0, 0.0, 10.0)); // At maximum
    ASSERT_FALSE(is_within(-1.0, 0.0, 10.0));
    ASSERT_FALSE(is_within(11.0, 0.0, 10.0));

    // Test sign function
    ASSERT_EQ(sign(5.0), 1);
    ASSERT_EQ(sign(-5.0), -1);
    ASSERT_EQ(sign(0.0), 0);

    // Test finite value check
    ASSERT_TRUE(is_finite_value(1.0));
    ASSERT_TRUE(is_finite_value(0.0));
    ASSERT_TRUE(is_finite_value(-1.0));
    ASSERT_FALSE(is_finite_value(1.0/0.0));  // Infinity
    ASSERT_FALSE(is_finite_value(0.0/0.0));  // NaN
}

/**
 * @brief Main test runner
 */
int main() {
    TEST_SUITE_START();

    RUN_TEST(test_safe_div);
    RUN_TEST(test_is_zero);
    RUN_TEST(test_is_equal);
    RUN_TEST(test_comparisons);
    RUN_TEST(test_utilities);

    TEST_SUITE_END();
}