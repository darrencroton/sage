#ifndef UTIL_NUMERIC_H
#define UTIL_NUMERIC_H

#include "constants.h"
#include <math.h>
#include <stdbool.h>

/**
 * @file    util_numeric.h
 * @brief   Utility functions for numerical stability and safer floating-point
 * operations
 *
 * This file provides utility functions to improve numerical stability in
 * the SAGE codebase by offering safer floating-point comparison operations,
 * division checks, and value validation functions. Using these utilities
 * instead of direct floating-point operations helps avoid common numerical
 * issues.
 */

/**
 * @brief   Checks if a value is effectively zero (within EPSILON_SMALL)
 *
 * @param   x   The value to check
 * @return  true if |x| < EPSILON_SMALL, false otherwise
 */
bool is_zero(double x);

/**
 * @brief   Checks if two values are equal within EPSILON_MEDIUM
 *
 * @param   x   First value to compare
 * @param   y   Second value to compare
 * @return  true if |x-y| < EPSILON_MEDIUM, false otherwise
 */
bool is_equal(double x, double y);

/**
 * @brief   Checks if x is definitely greater than y (accounting for
 * floating-point error)
 *
 * @param   x   First value to compare
 * @param   y   Second value to compare
 * @return  true if x > y + EPSILON_SMALL, false otherwise
 */
bool is_greater(double x, double y);

/**
 * @brief   Checks if x is definitely less than y (accounting for floating-point
 * error)
 *
 * @param   x   First value to compare
 * @param   y   Second value to compare
 * @return  true if x < y - EPSILON_SMALL, false otherwise
 */
bool is_less(double x, double y);

/**
 * @brief   Checks if x is greater than or equal to y (accounting for
 * floating-point error)
 *
 * @param   x   First value to compare
 * @param   y   Second value to compare
 * @return  true if x >= y - EPSILON_SMALL, false otherwise
 */
bool is_greater_or_equal(double x, double y);

/**
 * @brief   Checks if x is less than or equal to y (accounting for
 * floating-point error)
 *
 * @param   x   First value to compare
 * @param   y   Second value to compare
 * @return  true if x <= y + EPSILON_SMALL, false otherwise
 */
bool is_less_or_equal(double x, double y);

/**
 * @brief   Checks if a value is within a specified range (inclusive)
 *
 * @param   x     Value to check
 * @param   min   Minimum acceptable value
 * @param   max   Maximum acceptable value
 * @return  true if min <= x <= max (accounting for floating-point error), false
 * otherwise
 */
bool is_within(double x, double min, double max);

/**
 * @brief   Performs division with protection against division by zero
 *
 * @param   num           Numerator
 * @param   denom         Denominator
 * @param   default_val   Value to return if denominator is zero
 * @return  num/denom if denom is not zero, default_val otherwise
 */
double safe_div(double num, double denom, double default_val);

/**
 * @brief   Clamps a value between minimum and maximum bounds
 *
 * @param   val   Value to clamp
 * @param   min   Minimum bound
 * @param   max   Maximum bound
 * @return  Value clamped to the range [min, max]
 */
double clamp(double val, double min, double max);

/**
 * @brief   Checks if a value is finite (not NaN or infinity)
 *
 * @param   x   Value to check
 * @return  true if x is a finite number, false otherwise
 */
bool is_finite_value(double x);

/**
 * @brief   Calculates the sign of a value (-1, 0, or 1)
 *
 * @param   x   Value to get sign of
 * @return  -1 if x < 0, 0 if x is zero, 1 if x > 0
 */
int sign(double x);

#endif /* UTIL_NUMERIC_H */
