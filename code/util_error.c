/**
 * @file    error_handling.c
 * @brief   Implementation of the SAGE error handling and logging system
 *
 * This file implements a comprehensive error handling and logging system for
 * the SAGE model. It provides functions for logging messages at different
 * severity levels (debug, info, warning, error, fatal), with configurable
 * verbosity and output destination.
 *
 * The system supports:
 * - Multiple log levels with filtering
 * - Context-rich messages (timestamp, file, function, line)
 * - Configurable output destination
 * - Automatic newline handling
 * - Immediate flushing for critical messages
 *
 * Key functions:
 * - initialize_error_handling(): Sets up the logging system
 * - set_log_level(): Controls verbosity
 * - set_log_output(): Redirects log output
 * - log_message(): Core logging function
 *
 * This system improves error reporting and debugging capabilities,
 * replacing the old error handling approach based on ABORT/myexit.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "core_proto.h"
#include "util_error.h"

// Default log level: show everything except debug messages
static LogLevel current_log_level = LOG_LEVEL_INFO;

// Default output file pointer: stderr for warnings and errors, stdout for info
// and debug
static FILE *log_output = NULL;

// Level names for printing
static const char *level_names[] = {"DEBUG", "INFO", "WARNING", "ERROR",
                                    "FATAL"};

// I/O error code names for printing
static const char *io_error_names[] = {"NONE",
                                       "FILE_NOT_FOUND",
                                       "PERMISSION_DENIED",
                                       "READ_FAILED",
                                       "WRITE_FAILED",
                                       "SEEK_FAILED",
                                       "INVALID_HEADER",
                                       "VERSION_MISMATCH",
                                       "ENDIANNESS",
                                       "FORMAT",
                                       "BUFFER",
                                       "EOF",
                                       "CLOSE_FAILED",
                                       "HDF5"};

/**
 * @brief   Initializes the error handling and logging system
 *
 * @param   min_level      Minimum log level to display
 * @param   output_file    File handle for log output (NULL for auto)
 *
 * This function sets up the error handling system with the specified
 * minimum log level and output destination. Messages below the minimum
 * level will be filtered out. If output_file is NULL, logs will be sent
 * to stdout for debug/info messages and stderr for warnings/errors.
 *
 * The function also logs an initialization message at INFO level to
 * confirm the system is working correctly.
 */
void initialize_error_handling(LogLevel min_level, FILE *output_file) {
  set_log_level(min_level);
  set_log_output(output_file);

  // Log that the error handling system has been initialized
  INFO_LOG("Error handling system initialized. Log level set to %s",
           level_names[min_level]);
}

/**
 * @brief   Gets the string representation of a log level
 *
 * @param   level    The log level to convert to string
 * @return  String representation of the log level
 *
 * This function converts a LogLevel enum value to its corresponding
 * string representation (e.g., LOG_LEVEL_ERROR -> "ERROR"). If the
 * level is outside the valid range, it returns "UNKNOWN".
 *
 * The returned string is statically allocated and should not be freed.
 */
const char *get_log_level_name(LogLevel level) {
  if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL) {
    return "UNKNOWN";
  }
  return level_names[level];
}

/**
 * @brief   Gets the string representation of an I/O error code
 *
 * @param   code    The I/O error code to convert to string
 * @return  String representation of the error code
 *
 * This function converts an IOErrorCode enum value to its corresponding
 * string representation (e.g., IO_ERROR_READ_FAILED -> "READ_FAILED").
 * If the code is outside the valid range, it returns "UNKNOWN".
 *
 * The returned string is statically allocated and should not be freed.
 */
const char *get_io_error_name(IOErrorCode code) {
  if (code < IO_ERROR_NONE || code > IO_ERROR_HDF5) {
    return "UNKNOWN";
  }
  return io_error_names[code];
}

/**
 * @brief   Sets the minimum log level to display
 *
 * @param   min_level    New minimum log level
 *
 * This function changes the filtering level for log messages.
 * Messages with a level lower than min_level will be suppressed.
 * This allows runtime control of log verbosity.
 *
 * Log levels in order of increasing severity:
 * - LOG_LEVEL_DEBUG   (most verbose)
 * - LOG_LEVEL_INFO
 * - LOG_LEVEL_WARNING
 * - LOG_LEVEL_ERROR
 * - LOG_LEVEL_FATAL   (least verbose)
 */
void set_log_level(LogLevel min_level) { current_log_level = min_level; }

/**
 * @brief   Sets the output file for logging
 *
 * @param   output_file    New file handle for log output
 * @return  Previous output file handle
 *
 * This function changes where log messages are written. If output_file
 * is NULL, the system will automatically use stdout for debug/info
 * messages and stderr for warnings/errors.
 *
 * The function returns the previous output file handle, allowing it
 * to be restored later if needed.
 */
FILE *set_log_output(FILE *output_file) {
  FILE *old_output = log_output;
  log_output = output_file;
  return old_output;
}

/**
 * @brief   Central logging function
 *
 * @param   level     Severity level of the message
 * @param   file      Source file where logging occurs
 * @param   func      Function where logging occurs
 * @param   line      Line number where logging occurs
 * @param   format    Printf-style format string
 * @param   ...       Variable arguments for format string
 *
 * This function implements the core logging functionality. It formats
 * a message with context information (timestamp, level, file, function,
 * line) and writes it to the appropriate output. Messages below the
 * current minimum log level are silently ignored.
 *
 * The function automatically adds a newline if not present in the
 * format string, and immediately flushes the output for error and
 * fatal messages to ensure they are visible even if the program
 * terminates abnormally.
 *
 * Note: This function is not usually called directly; instead, use
 * the DEBUG_LOG, INFO_LOG, WARNING_LOG, ERROR_LOG, and FATAL_ERROR
 * macros defined in error_handling.h.
 */
void log_message(LogLevel level, const char *file, const char *func, int line,
                 const char *format, ...) {
  // Skip if below current log level
  if (level < current_log_level) {
    return;
  }

  // Get current time
  time_t now;
  time(&now);
  char time_str[20];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

  // Choose output stream based on message level
  FILE *output = log_output;
  if (output == NULL) {
    output = (level >= LOG_LEVEL_WARNING) ? stderr : stdout;
  }

  // Print header with time, level, file, function, and line
  fprintf(output, "[%s] %s - %s:%s:%d - ", time_str, level_names[level], file,
          func, line);

  // Print the actual message with variable arguments
  va_list args;
  va_start(args, format);
  vfprintf(output, format, args);
  va_end(args);

  // Add newline if not already present
  if (format[strlen(format) - 1] != '\n') {
    fprintf(output, "\n");
  }

  // Flush output immediately for errors and fatal messages
  if (level >= LOG_LEVEL_ERROR) {
    fflush(output);
  }
}

/**
 * @brief   I/O-specific logging function
 *
 * @param   level      Severity level of the message
 * @param   code       I/O error code
 * @param   file       Source file where logging occurs
 * @param   func       Function where logging occurs
 * @param   line       Line number where logging occurs
 * @param   operation  Type of I/O operation (e.g., "read", "write", "open")
 * @param   filename   Name of the file being operated on
 * @param   format     Printf-style format string for additional details
 * @param   ...        Variable arguments for format string
 *
 * This function extends the core logging functionality with I/O-specific
 * context information. It formats a message with operation type, filename,
 * and error code in addition to the standard context information.
 *
 * The function is especially useful for standardizing I/O error reporting
 * throughout the codebase. It follows the same filtering and output rules
 * as the core log_message() function.
 *
 * Note: This function is not usually called directly; instead, use
 * the IO_DEBUG_LOG, IO_INFO_LOG, IO_WARNING_LOG, IO_ERROR_LOG, and
 * IO_FATAL_ERROR macros defined in error_handling.h.
 */
void log_io_error(LogLevel level, IOErrorCode code, const char *file,
                  const char *func, int line, const char *operation,
                  const char *filename, const char *format, ...) {
  // Skip if below current log level
  if (level < current_log_level) {
    return;
  }

  // Get current time
  time_t now;
  time(&now);
  char time_str[20];
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

  // Choose output stream based on message level
  FILE *output = log_output;
  if (output == NULL) {
    output = (level >= LOG_LEVEL_WARNING) ? stderr : stdout;
  }

  // Print header with time, level, file, function, and line
  fprintf(output, "[%s] %s - %s:%s:%d - ", time_str, level_names[level], file,
          func, line);

  // Print I/O-specific information: operation, filename, and error code
  fprintf(output, "[I/O %s, file: '%s', error: %s] ", operation,
          filename ? filename : "?", get_io_error_name(code));

  // Print the additional details with variable arguments
  va_list args;
  va_start(args, format);
  vfprintf(output, format, args);
  va_end(args);

  // Add newline if not already present
  if (format[strlen(format) - 1] != '\n') {
    fprintf(output, "\n");
  }

  // Flush output immediately for errors and fatal messages
  if (level >= LOG_LEVEL_ERROR) {
    fflush(output);
  }
}
