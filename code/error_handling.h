#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdio.h>

// Error severity levels
typedef enum {
    LOG_LEVEL_DEBUG,   // Detailed debugging information
    LOG_LEVEL_INFO,    // General informational messages
    LOG_LEVEL_WARNING, // Warnings that don't stop execution
    LOG_LEVEL_ERROR,   // Recoverable errors
    LOG_LEVEL_FATAL    // Unrecoverable errors that terminate execution
} LogLevel;

// Function prototypes
void initialize_error_handling(LogLevel min_level, FILE *output_file);
void log_message(LogLevel level, const char *file, const char *func, int line, const char *format, ...);
void set_log_level(LogLevel min_level);
FILE* set_log_output(FILE *output_file);

// Convenience macros
#define DEBUG_LOG(...)   log_message(LOG_LEVEL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define INFO_LOG(...)    log_message(LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define WARNING_LOG(...) log_message(LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define ERROR_LOG(...)   log_message(LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define FATAL_ERROR(...) do { \
    log_message(LOG_LEVEL_FATAL, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); \
    myexit(1); \
} while(0)

// Get string representation of log level
const char* get_log_level_name(LogLevel level);

#endif /* ERROR_HANDLING_H */
