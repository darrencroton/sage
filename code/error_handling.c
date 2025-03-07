#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "error_handling.h"
#include "core_proto.h"

// Default log level: show everything except debug messages
static LogLevel current_log_level = LOG_LEVEL_INFO;

// Default output file pointer: stderr for warnings and errors, stdout for info and debug
static FILE *log_output = NULL;

// Level names for printing
static const char *level_names[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "FATAL"
};

// Initialize error handling system
void initialize_error_handling(LogLevel min_level, FILE *output_file) {
    set_log_level(min_level);
    set_log_output(output_file);
    
    // Log that the error handling system has been initialized
    INFO_LOG("Error handling system initialized. Log level set to %s", level_names[min_level]);
}

// Get string representation of log level
const char* get_log_level_name(LogLevel level) {
    if (level < LOG_LEVEL_DEBUG || level > LOG_LEVEL_FATAL) {
        return "UNKNOWN";
    }
    return level_names[level];
}

// Set the minimum log level to display
void set_log_level(LogLevel min_level) {
    current_log_level = min_level;
}

// Set the output file for logging
FILE* set_log_output(FILE *output_file) {
    FILE *old_output = log_output;
    log_output = output_file;
    return old_output;
}

// Central logging function
void log_message(LogLevel level, const char *file, const char *func, int line, const char *format, ...) {
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
    fprintf(output, "[%s] %s - %s:%s:%d - ", 
            time_str, level_names[level], file, func, line);

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
