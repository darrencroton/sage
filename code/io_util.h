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

#include <stdio.h>
#include <stdint.h>

/* Endianness definitions */
#define SAGE_LITTLE_ENDIAN 0
#define SAGE_BIG_ENDIAN    1

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
#define SAGE_MAGIC_NUMBER 0x53414745  /* "SAGE" in ASCII */

/* Current binary file format version */
#define SAGE_FILE_VERSION 1

/* File header structure for binary files */
struct SAGEFileHeader {
    uint32_t magic;       /* Magic number for identification (SAGE_MAGIC_NUMBER) */
    uint8_t  version;     /* File format version */
    uint8_t  endianness;  /* File endianness (0=little, 1=big) */
    uint16_t reserved;    /* Reserved for future use */
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
void* swap_bytes_if_needed(void* data, size_t size, size_t count, int file_endian);

/* File format utilities - for future implementation */
int write_sage_header(FILE* file, int endianness);
int read_sage_header(FILE* file, struct SAGEFileHeader* header);
int check_file_compatibility(const struct SAGEFileHeader* header);

#endif /* IO_UTIL_H */