/**
 * @file    util_version.h
 * @brief   Functionality for tracking SAGE version information
 *
 * This file provides function declarations for generating and saving version
 * and runtime information to a metadata file in the SAGE output directory.
 * This helps with reproducibility by ensuring that output data can always
 * be traced back to the exact code version that produced it.
 */

#ifndef UTIL_VERSION_H
#define UTIL_VERSION_H

/**
 * @brief   Creates a version metadata file in the specified output directory
 *
 * @param   output_dir         Path to the output directory
 * @param   parameter_file     Path to the parameter file used for this run
 *
 * This function creates a JSON metadata file in the output directory
 * containing information about the SAGE version, build environment,
 * git commit details, runtime parameters, and system information.
 *
 * If git information cannot be retrieved, those fields will be marked
 * as unavailable in the output.
 *
 * @return  0 on success, non-zero on error
 */
int create_version_metadata(const char *output_dir, const char *parameter_file);

#endif /* UTIL_VERSION_H */
