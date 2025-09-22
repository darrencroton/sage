/**
 * @file    test_utils.c
 * @brief   Test utility functions for SAGE unit tests
 *
 * This file provides minimal utility functions required for testing
 * without dependencies on the full SAGE infrastructure.
 */

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Simple exit function for tests (replaces main.c's myexit)
 * @param signum Exit code
 */
void myexit(int signum) {
    printf("Test exiting with code %d\n", signum);
    exit(signum);
}