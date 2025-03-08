/**
 * @file    util_integration.h
 * @brief   Numerical integration utilities to replace GSL dependency
 *
 * This file provides replacements for GSL numerical integration functions
 * used in the SAGE codebase. It implements highly accurate numerical integration
 * methods to ensure the same level of precision as the original GSL implementations.
 */

#ifndef UTIL_INTEGRATION_H
#define UTIL_INTEGRATION_H

/* Constants for integration methods (for compatibility with GSL) */
#define GSL_INTEG_GAUSS15  1
#define GSL_INTEG_GAUSS21  2
#define GSL_INTEG_GAUSS31  3
#define GSL_INTEG_GAUSS41  4
#define GSL_INTEG_GAUSS51  5
#define GSL_INTEG_GAUSS61  6

/**
 * @brief Function pointer type for the integrand function
 */
typedef double (*integrand_func_t)(double x, void *params);

/**
 * @brief Structure to hold integration function and parameters
 */
typedef struct {
    integrand_func_t function; /**< Pointer to the integrand function */
    void *params;             /**< Parameters to pass to the integrand function */
} integration_function_t;

/**
 * @brief Integration workspace structure (minimal version)
 * 
 * This is a simplified version of the integration workspace.
 * We don't actually use its contents, but keep it for API compatibility.
 */
typedef struct {
    int size;        /**< Workspace size */
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
 * @brief Adaptive integration using Simpson's rule
 *
 * This function replaces gsl_integration_qag for high-accuracy integration.
 * It implements adaptive integration using Simpson's rule to achieve
 * the same level of accuracy as GSL for smooth functions.
 *
 * @param f        Integration function structure
 * @param a        Lower integration limit
 * @param b        Upper integration limit
 * @param epsabs   Absolute error tolerance
 * @param epsrel   Relative error tolerance
 * @param limit    Maximum number of subintervals
 * @param key      Integration rule (ignored in this implementation)
 * @param workspace Integration workspace
 * @param result   Pointer to store integration result
 * @param abserr   Pointer to store error estimate
 * @return Status code (0 for success)
 */
int integration_qag(
    integration_function_t *f,
    double a, double b,
    double epsabs, double epsrel,
    size_t limit,
    int key,
    integration_workspace_t *workspace,
    double *result, double *abserr
);

#endif /* UTIL_INTEGRATION_H */
