/**
 * @file    util_integration.h
 * @brief   Numerical integration utilities for SAGE
 *
 * This file provides highly accurate numerical integration functions used
 * in the SAGE codebase. It implements adaptive integration methods for
 * precise calculations of cosmological quantities.
 */

#include <stddef.h> // For size_t definition

#ifndef UTIL_INTEGRATION_H
#define UTIL_INTEGRATION_H

/* Constants for integration methods */
#define INTEG_GAUSS15 1
#define INTEG_GAUSS21 2
#define INTEG_GAUSS31 3
#define INTEG_GAUSS41 4
#define INTEG_GAUSS51 5
#define INTEG_GAUSS61 6

/**
 * @brief Function pointer type for the integrand function
 */
typedef double (*integrand_func_t)(double x, void *params);

/**
 * @brief Structure to hold integration function and parameters
 */
typedef struct {
  integrand_func_t function; /**< Pointer to the integrand function */
  void *params; /**< Parameters to pass to the integrand function */
} integration_function_t;

/**
 * @brief Integration workspace structure
 *
 * This structure holds the workspace for numerical integration.
 * Current implementation only tracks size.
 */
typedef struct {
  int size; /**< Workspace size */
} integration_workspace_t;

/**
 * @brief Allocate integration workspace (simplified)
 *
 * @param size Size parameter (not actually used)
 * @return Pointer to workspace
 */
integration_workspace_t *integration_workspace_alloc(size_t size);

/**
 * @brief Free integration workspace (simplified)
 *
 * @param workspace Pointer to workspace to free
 */
void integration_workspace_free(integration_workspace_t *workspace);

/**
 * @brief Adaptive integration for high-accuracy numerical integration
 *
 * Implements adaptive integration using Simpson's rule for high-accuracy
 * calculation of definite integrals. Suitable for smooth functions like
 * those encountered in cosmological calculations.
 *
 * @param f        Integration function structure
 * @param a        Lower integration limit
 * @param b        Upper integration limit
 * @param epsabs   Absolute error tolerance
 * @param epsrel   Relative error tolerance
 * @param limit    Maximum number of subintervals
 * @param key      Integration rule to use (see INTEG_* constants)
 * @param workspace Integration workspace
 * @param result   Pointer to store integration result
 * @param abserr   Pointer to store error estimate
 * @return Status code (0 for success)
 */
int integration_qag(integration_function_t *f, double a, double b,
                    double epsabs, double epsrel, size_t limit, int key,
                    integration_workspace_t *workspace, double *result,
                    double *abserr);

#endif /* UTIL_INTEGRATION_H */
