/**
 * @file    io_util.h
 * @brief   Utility functions for I/O operations
 *
 * This file provides utilities for handling input/output operations,
 * including endianness detection and conversion, and improved error handling.
 * These utilities ensure consistent cross-platform compatibility for binary
 * file formats and standardized error reporting.
 */

#ifndef IO_UTIL_H
#define IO_UTIL_H

#include <stdint.h>
#include <stdio.h>

/* Buffer management definitions */
#define IO_BUFFER_SMALL 8192   /* 8 KB - For small files or random access */
#define IO_BUFFER_MEDIUM 65536 /* 64 KB - Default buffer size */
#define IO_BUFFER_LARGE                                                        \
  262144 /* 256 KB - For sequential access to large files */
#define IO_BUFFER_XLARGE                                                       \
  1048576 /* 1 MB - For very large files or high bandwidth operations */
#define MAX_BUFFERED_FILES 16 /* Maximum number of buffered files */

/* Buffer access modes */
typedef enum {
  IO_BUFFER_READ,     /* Read-only buffer */
  IO_BUFFER_WRITE,    /* Write-only buffer */
  IO_BUFFER_READWRITE /* Read-write buffer */
} IOBufferMode;

/* I/O buffer structure */
typedef struct {
  void *buffer;      /* Buffer memory */
  size_t size;       /* Total buffer size */
  size_t position;   /* Current position in buffer */
  size_t valid_size; /* Amount of valid data in buffer */
  int dirty;         /* Whether buffer contains unsaved changes */
  FILE *file;        /* Associated file */
  IOBufferMode mode; /* Buffer mode (read, write, or both) */
  long file_offset;  /* File offset corresponding to buffer start */
} IOBuffer;

/* Endianness definitions */
#define SAGE_LITTLE_ENDIAN 0
#define SAGE_BIG_ENDIAN 1

/* Determine host endianness at compile time if possible */
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define SAGE_HOST_ENDIAN SAGE_LITTLE_ENDIAN
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SAGE_HOST_ENDIAN SAGE_BIG_ENDIAN
#elif defined(__LITTLE_ENDIAN__)
#define SAGE_HOST_ENDIAN SAGE_LITTLE_ENDIAN
#elif defined(__BIG_ENDIAN__)
#define SAGE_HOST_ENDIAN SAGE_BIG_ENDIAN
#else
/* Runtime detection as fallback - implemented in io_util.c */
int detect_host_endian(void);
#define SAGE_HOST_ENDIAN detect_host_endian()
#endif

/* Define the magic number for SAGE binary files */
#define SAGE_MAGIC_NUMBER 0x53414745 /* "SAGE" in ASCII */

/* Current binary file format version */
#define SAGE_FILE_VERSION 1

/* File header structure for binary files */
struct SAGEFileHeader {
  uint32_t magic;     /* Magic number for identification (SAGE_MAGIC_NUMBER) */
  uint8_t version;    /* File format version */
  uint8_t endianness; /* File endianness (0=little, 1=big) */
  uint16_t reserved;  /* Reserved for future use */
};

/* Function prototypes for endianness conversion */
uint16_t swap_uint16(uint16_t value);
uint32_t swap_uint32(uint32_t value);
uint64_t swap_uint64(uint64_t value);
int16_t swap_int16(int16_t value);
int32_t swap_int32(int32_t value);
int64_t swap_int64(int64_t value);
float swap_float(float value);
double swap_double(double value);

/* Endianness utilities */
int is_same_endian(int file_endian);
void *swap_bytes_if_needed(void *data, size_t size, size_t count,
                           int file_endian);

/* File format utilities */
int write_sage_header(FILE *file, int endianness);
int read_sage_header(FILE *file, struct SAGEFileHeader *header);
int check_file_compatibility(const struct SAGEFileHeader *header);
int check_headerless_file(FILE *file);
long get_file_size(FILE *file);

/* Buffer management functions */
IOBuffer *create_buffer(size_t size, IOBufferMode mode, FILE *file);
void free_buffer(IOBuffer *buffer);
int flush_buffer(IOBuffer *buffer);
int fill_buffer(IOBuffer *buffer, size_t min_fill);
size_t get_optimal_buffer_size(FILE *file);

/* Buffer registry management */
IOBuffer *get_buffer(FILE *file);
int register_buffer(FILE *file, IOBuffer *buffer);
int unregister_buffer(FILE *file);

/* Buffered I/O operations */
size_t buffered_read(IOBuffer *buffer, void *data, size_t size, size_t count);
size_t buffered_write(IOBuffer *buffer, const void *data, size_t size,
                      size_t count);
int buffered_seek(IOBuffer *buffer, long offset, int whence);

/* Buffered file operations */
FILE *buffered_fopen(const char *filename, const char *mode,
                     size_t buffer_size);
int buffered_fclose(FILE *file);
int buffered_flush(FILE *file);

#endif /* IO_UTIL_H */