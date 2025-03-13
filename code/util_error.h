#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>

/**
 * Error severity levels
 */
typedef enum {
  LOG_LEVEL_DEBUG, // Detailed debugging information (function tracing, variable
                   // values, etc.)
  LOG_LEVEL_INFO,  // General informational messages (processing milestones,
                   // configuration info)
  LOG_LEVEL_WARNING, // Warnings that don't stop execution (unusual but
                     // acceptable conditions)
  LOG_LEVEL_ERROR,   // Recoverable errors (operation failed but program can
                     // continue)
  LOG_LEVEL_FATAL    // Unrecoverable errors that terminate execution (critical
                     // failures)
} LogLevel;

/**
 * I/O-specific error codes
 */
typedef enum {
  IO_ERROR_NONE = 0,              // No error
  IO_ERROR_FILE_NOT_FOUND = 1,    // File not found or couldn't be opened
  IO_ERROR_PERMISSION_DENIED = 2, // Permission denied for file operation
  IO_ERROR_READ_FAILED = 3,       // Read operation failed
  IO_ERROR_WRITE_FAILED = 4,      // Write operation failed
  IO_ERROR_SEEK_FAILED = 5,       // Seek operation failed
  IO_ERROR_INVALID_HEADER = 6,    // Invalid or corrupted file header
  IO_ERROR_VERSION_MISMATCH = 7,  // File version incompatible
  IO_ERROR_ENDIANNESS = 8,        // Endianness-related error
  IO_ERROR_FORMAT = 9,            // General file format error
  IO_ERROR_BUFFER = 10,           // Buffer management error
  IO_ERROR_EOF = 11,              // Unexpected end of file
  IO_ERROR_CLOSE_FAILED = 12,     // Failed to close file
  IO_ERROR_HDF5 = 13              // HDF5-specific error
} IOErrorCode;

// General error handling function prototypes
void initialize_error_handling(LogLevel min_level, FILE *output_file);
void log_message(LogLevel level, const char *file, const char *func, int line,
                 const char *format, ...);
void set_log_level(LogLevel min_level);
FILE *set_log_output(FILE *output_file);

// I/O-specific error handling function prototypes
const char *get_io_error_name(IOErrorCode code);
void log_io_error(LogLevel level, IOErrorCode code, const char *file,
                  const char *func, int line, const char *operation,
                  const char *filename, const char *format, ...);

// General logging convenience macros
#define DEBUG_LOG(...)                                                         \
  log_message(LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define INFO_LOG(...)                                                          \
  log_message(LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define WARNING_LOG(...)                                                       \
  log_message(LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define ERROR_LOG(...)                                                         \
  log_message(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define FATAL_ERROR(...)                                                       \
  do {                                                                         \
    log_message(LOG_LEVEL_FATAL, __FILE__, __FUNCTION__, __LINE__,             \
                __VA_ARGS__);                                                  \
    myexit(1);                                                                 \
  } while (0)

// I/O-specific logging macros
#define IO_DEBUG_LOG(code, op, filename, ...)                                  \
  log_io_error(LOG_LEVEL_DEBUG, code, __FILE__, __FUNCTION__, __LINE__, op,    \
               filename, __VA_ARGS__)
#define IO_INFO_LOG(code, op, filename, ...)                                   \
  log_io_error(LOG_LEVEL_INFO, code, __FILE__, __FUNCTION__, __LINE__, op,     \
               filename, __VA_ARGS__)
#define IO_WARNING_LOG(code, op, filename, ...)                                \
  log_io_error(LOG_LEVEL_WARNING, code, __FILE__, __FUNCTION__, __LINE__, op,  \
               filename, __VA_ARGS__)
#define IO_ERROR_LOG(code, op, filename, ...)                                  \
  log_io_error(LOG_LEVEL_ERROR, code, __FILE__, __FUNCTION__, __LINE__, op,    \
               filename, __VA_ARGS__)
#define IO_FATAL_ERROR(code, op, filename, ...)                                \
  do {                                                                         \
    log_io_error(LOG_LEVEL_FATAL, code, __FILE__, __FUNCTION__, __LINE__, op,  \
                 filename, __VA_ARGS__);                                       \
    myexit(1);                                                                 \
  } while (0)

// String representation functions
const char *get_log_level_name(LogLevel level);

#endif /* ERROR_HANDLING_H */
