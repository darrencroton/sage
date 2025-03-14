/**
 * @file    util_numeric.c
 * @brief   Implementation of utility functions for numerical stability
 *
 * This file implements utility functions to improve numerical stability in
 * floating-point operations throughout the SAGE codebase. It provides
 * safer alternatives to direct comparison operations, division, and
 * value bounds checking.
 */

#include "util_numeric.h"
#include "constants.h"
#include "util_error.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

/* Check if value is effectively zero (within EPSILON_SMALL) */
bool is_zero(double x) { return fabs(x) < EPSILON_SMALL; }

/* Check if two values are equal within EPSILON_MEDIUM */
bool is_equal(double x, double y) { return fabs(x - y) < EPSILON_MEDIUM; }

/* Check if x is definitely greater than y */
bool is_greater(double x, double y) { return x > y + EPSILON_SMALL; }

/* Check if x is definitely less than y */
bool is_less(double x, double y) { return x < y - EPSILON_SMALL; }

/* Check if x is greater than or equal to y */
bool is_greater_or_equal(double x, double y) { return x >= y - EPSILON_SMALL; }

/* Check if x is less than or equal to y */
bool is_less_or_equal(double x, double y) { return x <= y + EPSILON_SMALL; }

/* Check if value is within range [min, max] */
bool is_within(double x, double min, double max) {
  return is_greater_or_equal(x, min) && is_less_or_equal(x, max);
}

/* Perform division with protection against division by zero */
double safe_div(double num, double denom, double default_val) {
  if (is_zero(denom)) {
    return default_val;
  }
  return num / denom;
}

/* Clamp value between minimum and maximum bounds */
double clamp(double val, double min, double max) {
  if (val < min)
    return min;
  if (val > max)
    return max;
  return val;
}

/* Check if value is finite (not NaN or infinity) */
bool is_finite_value(double x) { return isfinite(x); }

/* Calculate sign of value (-1, 0, or 1) */
int sign(double x) {
  if (is_zero(x))
    return 0;
  return x < 0 ? -1 : 1;
}
