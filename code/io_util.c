/**
 * @file    io_util.c
 * @brief   Implementation of I/O utility functions
 *
 * This file implements utilities for handling input/output operations,
 * including endianness detection and conversion. These utilities ensure
 * consistent cross-platform compatibility for binary file formats.
 */

#include <stdlib.h>
#include <string.h>

#include "io_util.h"
#include "util_error.h"

/**
 * @brief   Detects the host system's endianness at runtime
 *
 * @return  SAGE_BIG_ENDIAN or SAGE_LITTLE_ENDIAN
 *
 * This function is used as a fallback when compile-time detection
 * is not possible. It determines the endianness by examining how a
 * multi-byte integer is stored in memory.
 */
int detect_host_endian(void) {
    union {
        uint32_t i;
        char c[4];
    } test = {0x01020304};
    
    return (test.c[0] == 1) ? SAGE_BIG_ENDIAN : SAGE_LITTLE_ENDIAN;
}

/**
 * @brief   Swaps the bytes in a 16-bit unsigned integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
uint16_t swap_uint16(uint16_t value) {
    return ((value & 0x00FF) << 8) | 
           ((value & 0xFF00) >> 8);
}

/**
 * @brief   Swaps the bytes in a 32-bit unsigned integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
uint32_t swap_uint32(uint32_t value) {
    return ((value & 0x000000FF) << 24) | 
           ((value & 0x0000FF00) << 8)  | 
           ((value & 0x00FF0000) >> 8)  | 
           ((value & 0xFF000000) >> 24);
}

/**
 * @brief   Swaps the bytes in a 64-bit unsigned integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
uint64_t swap_uint64(uint64_t value) {
    return ((value & 0x00000000000000FFULL) << 56) | 
           ((value & 0x000000000000FF00ULL) << 40) | 
           ((value & 0x0000000000FF0000ULL) << 24) | 
           ((value & 0x00000000FF000000ULL) << 8)  | 
           ((value & 0x000000FF00000000ULL) >> 8)  | 
           ((value & 0x0000FF0000000000ULL) >> 24) | 
           ((value & 0x00FF000000000000ULL) >> 40) | 
           ((value & 0xFF00000000000000ULL) >> 56);
}

/**
 * @brief   Swaps the bytes in a 16-bit signed integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
int16_t swap_int16(int16_t value) {
    return (int16_t)swap_uint16((uint16_t)value);
}

/**
 * @brief   Swaps the bytes in a 32-bit signed integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
int32_t swap_int32(int32_t value) {
    return (int32_t)swap_uint32((uint32_t)value);
}

/**
 * @brief   Swaps the bytes in a 64-bit signed integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
int64_t swap_int64(int64_t value) {
    return (int64_t)swap_uint64((uint64_t)value);
}

/**
 * @brief   Swaps the bytes in a 32-bit floating point value
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
float swap_float(float value) {
    union {
        float f;
        uint32_t i;
    } converter;
    
    converter.f = value;
    converter.i = swap_uint32(converter.i);
    
    return converter.f;
}

/**
 * @brief   Swaps the bytes in a 64-bit floating point value
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
double swap_double(double value) {
    union {
        double d;
        uint64_t i;
    } converter;
    
    converter.d = value;
    converter.i = swap_uint64(converter.i);
    
    return converter.d;
}

/**
 * @brief   Checks if the file endianness matches the host endianness
 *
 * @param   file_endian   The endianness of the file (SAGE_LITTLE_ENDIAN or SAGE_BIG_ENDIAN)
 * @return  1 if same endianness, 0 if different
 */
int is_same_endian(int file_endian) {
    return file_endian == SAGE_HOST_ENDIAN;
}

/**
 * @brief   Swaps bytes in memory if host and file endianness differ
 *
 * @param   data         Pointer to the data to swap
 * @param   size         Size of each element in bytes
 * @param   count        Number of elements
 * @param   file_endian  Endianness of the file
 * @return  Pointer to the input data (modified in place)
 * 
 * This function checks if byte swapping is needed and performs it if necessary.
 * It supports common data sizes (2, 4, and 8 bytes). The data is modified in place.
 */
void* swap_bytes_if_needed(void* data, size_t size, size_t count, int file_endian) {
    /* Return early if no conversion is needed */
    if (is_same_endian(file_endian) || data == NULL || count == 0) {
        return data;
    }
    
    /* Cast to byte array for easier manipulation */
    unsigned char* bytes = (unsigned char*)data;
    unsigned char tmp;
    size_t i, j;
    
    /* Swap bytes based on element size */
    switch (size) {
        case 2: /* 16-bit values */
            for (i = 0; i < count; i++) {
                tmp = bytes[i*2];
                bytes[i*2] = bytes[i*2 + 1];
                bytes[i*2 + 1] = tmp;
            }
            break;
            
        case 4: /* 32-bit values */
            for (i = 0; i < count; i++) {
                for (j = 0; j < 2; j++) {
                    tmp = bytes[i*4 + j];
                    bytes[i*4 + j] = bytes[i*4 + 3 - j];
                    bytes[i*4 + 3 - j] = tmp;
                }
            }
            break;
            
        case 8: /* 64-bit values */
            for (i = 0; i < count; i++) {
                for (j = 0; j < 4; j++) {
                    tmp = bytes[i*8 + j];
                    bytes[i*8 + j] = bytes[i*8 + 7 - j];
                    bytes[i*8 + 7 - j] = tmp;
                }
            }
            break;
            
        default:
            WARNING_LOG("Unsupported element size for byte swapping: %zu bytes", size);
            break;
    }
    
    return data;
}

/**
 * @brief   Writes a SAGE file header to a binary file
 *
 * @param   file        File pointer to write to
 * @param   endianness  Endianness to record in the header
 * @return  0 on success, non-zero on error
 *
 * This function writes a standardized header to binary files,
 * including a magic number, version, and endianness information.
 * 
 * This function is a placeholder for Phase 2 implementation.
 */
int write_sage_header(FILE* file, int endianness) {
    /* This is a stub for Phase 2 implementation */
    WARNING_LOG("write_sage_header() is not yet implemented");
    return 0;
}

/**
 * @brief   Reads a SAGE file header from a binary file
 *
 * @param   file    File pointer to read from
 * @param   header  Pointer to header structure to fill
 * @return  0 on success, non-zero on error
 *
 * This function reads and validates a SAGE file header.
 * 
 * This function is a placeholder for Phase 2 implementation.
 */
int read_sage_header(FILE* file, struct SAGEFileHeader* header) {
    /* This is a stub for Phase 2 implementation */
    WARNING_LOG("read_sage_header() is not yet implemented");
    return 0;
}

/**
 * @brief   Checks if a file is compatible with the current SAGE version
 *
 * @param   header  Pointer to the file header
 * @return  0 if compatible, non-zero if incompatible
 *
 * This function verifies the magic number and checks version compatibility.
 * 
 * This function is a placeholder for Phase 2 implementation.
 */
int check_file_compatibility(const struct SAGEFileHeader* header) {
    /* This is a stub for Phase 2 implementation */
    WARNING_LOG("check_file_compatibility() is not yet implemented");
    return 0;
}
