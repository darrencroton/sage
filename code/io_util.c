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
 * The header is always written in the host endianness to avoid
 * the need for conversion when reading the header itself.
 */
int write_sage_header(FILE* file, int endianness) {
    struct SAGEFileHeader header;
    size_t written;
    
    if (file == NULL) {
        IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "write_header", NULL, 
                    "Cannot write header to NULL file pointer");
        return IO_ERROR_FILE_NOT_FOUND;
    }
    
    /* Initialize header */
    header.magic = SAGE_MAGIC_NUMBER;
    header.version = SAGE_FILE_VERSION;
    header.endianness = (uint8_t)endianness;
    header.reserved = 0;
    
    /* Write header to file */
    written = fwrite(&header, sizeof(struct SAGEFileHeader), 1, file);
    if (written != 1) {
        IO_ERROR_LOG(IO_ERROR_WRITE_FAILED, "write_header", NULL, 
                    "Failed to write file header (%zu bytes written)", written);
        return IO_ERROR_WRITE_FAILED;
    }
    
    DEBUG_LOG("Successfully wrote SAGE file header (magic: 0x%08X, version: %d, endianness: %d)",
              header.magic, header.version, header.endianness);
    
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
 * It checks for the magic number to identify valid SAGE files.
 * The function returns appropriate error codes if the file
 * doesn't have a valid header.
 */
int read_sage_header(FILE* file, struct SAGEFileHeader* header) {
    size_t read_items;
    long current_pos;
    
    if (file == NULL) {
        IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "read_header", NULL, 
                    "Cannot read header from NULL file pointer");
        return IO_ERROR_FILE_NOT_FOUND;
    }
    
    if (header == NULL) {
        IO_ERROR_LOG(IO_ERROR_INVALID_HEADER, "read_header", NULL, 
                    "Cannot read header into NULL pointer");
        return IO_ERROR_INVALID_HEADER;
    }
    
    /* Save the current file position for potential rollback */
    current_pos = ftell(file);
    if (current_pos < 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "read_header", NULL, 
                    "Failed to get current file position");
        return IO_ERROR_SEEK_FAILED;
    }
    
    /* Read header from file */
    read_items = fread(header, sizeof(struct SAGEFileHeader), 1, file);
    if (read_items != 1) {
        /* Restore file position on error */
        fseek(file, current_pos, SEEK_SET);
        IO_ERROR_LOG(IO_ERROR_READ_FAILED, "read_header", NULL, 
                    "Failed to read file header (%zu items read)", read_items);
        return IO_ERROR_READ_FAILED;
    }
    
    /* Check for magic number */
    if (header->magic != SAGE_MAGIC_NUMBER) {
        /* Restore file position if this isn't a SAGE file */
        fseek(file, current_pos, SEEK_SET);
        IO_WARNING_LOG(IO_ERROR_INVALID_HEADER, "read_header", NULL, 
                      "Invalid file header magic number: 0x%08X (expected: 0x%08X)",
                      header->magic, SAGE_MAGIC_NUMBER);
        return IO_ERROR_INVALID_HEADER;
    }
    
    DEBUG_LOG("Successfully read SAGE file header (magic: 0x%08X, version: %d, endianness: %d)",
              header->magic, header->version, header->endianness);
    
    return 0;
}

/**
 * @brief   Checks if a file is compatible with the current SAGE version
 *
 * @param   header  Pointer to the file header
 * @return  0 if compatible, non-zero if incompatible
 *
 * This function verifies the magic number and checks version compatibility.
 * It implements a backwards-compatibility policy where newer SAGE versions
 * can read files created by older versions, but not vice versa.
 */
int check_file_compatibility(const struct SAGEFileHeader* header) {
    if (header == NULL) {
        IO_ERROR_LOG(IO_ERROR_INVALID_HEADER, "check_compatibility", NULL, 
                    "Cannot check compatibility with NULL header");
        return IO_ERROR_INVALID_HEADER;
    }
    
    /* Verify magic number */
    if (header->magic != SAGE_MAGIC_NUMBER) {
        IO_ERROR_LOG(IO_ERROR_INVALID_HEADER, "check_compatibility", NULL, 
                    "Invalid file header magic number: 0x%08X (expected: 0x%08X)",
                    header->magic, SAGE_MAGIC_NUMBER);
        return IO_ERROR_INVALID_HEADER;
    }
    
    /* Check version compatibility 
     * Policy: newer SAGE versions can read older file formats, but
     * older SAGE versions cannot read newer file formats */
    if (header->version > SAGE_FILE_VERSION) {
        IO_ERROR_LOG(IO_ERROR_VERSION_MISMATCH, "check_compatibility", NULL, 
                    "File format version %d is newer than this SAGE version (%d)",
                    header->version, SAGE_FILE_VERSION);
        return IO_ERROR_VERSION_MISMATCH;
    }
    
    /* Validate endianness field */
    if (header->endianness != SAGE_LITTLE_ENDIAN && 
        header->endianness != SAGE_BIG_ENDIAN) {
        IO_ERROR_LOG(IO_ERROR_ENDIANNESS, "check_compatibility", NULL, 
                    "Invalid endianness value in file header: %d",
                    header->endianness);
        return IO_ERROR_ENDIANNESS;
    }
    
    return 0;
}

/**
 * @brief   Checks if a file is a headerless legacy file
 *
 * @param   file    File pointer to check
 * @return  1 if headerless, 0 if has header, negative value on error
 *
 * This function attempts to determine if a file is a legacy file
 * without a SAGE header. It looks at file size and tests for integer
 * patterns at the beginning of the file that would indicate a headerless file.
 */
int check_headerless_file(FILE* file) {
    long original_pos, file_size;
    int ntrees, tothalos;
    size_t read_items;
    
    if (file == NULL) {
        IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "check_headerless", NULL, 
                   "Cannot check NULL file pointer");
        return -IO_ERROR_FILE_NOT_FOUND;
    }
    
    /* Save current file position */
    original_pos = ftell(file);
    if (original_pos < 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "check_headerless", NULL, 
                   "Failed to get current file position");
        return -IO_ERROR_SEEK_FAILED;
    }
    
    /* Get file size */
    file_size = get_file_size(file);
    if (file_size < 0) {
        /* Error already logged by get_file_size */
        fseek(file, original_pos, SEEK_SET);
        return file_size; /* Return negative error code */
    }
    
    /* If file is too small to even contain ntrees and tothalos, it's invalid */
    if (file_size < (long)(2 * sizeof(int))) {
        IO_ERROR_LOG(IO_ERROR_FORMAT, "check_headerless", NULL, 
                   "File too small to be a valid SAGE file (%ld bytes)", file_size);
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_FORMAT;
    }
    
    /* Rewind to beginning of file */
    if (fseek(file, 0, SEEK_SET) != 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "check_headerless", NULL, 
                   "Failed to seek to beginning of file");
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_SEEK_FAILED;
    }
    
    /* Check for SAGE header first */
    struct SAGEFileHeader header;
    read_items = fread(&header, sizeof(struct SAGEFileHeader), 1, file);
    if (read_items != 1) {
        IO_ERROR_LOG(IO_ERROR_READ_FAILED, "check_headerless", NULL, 
                   "Failed to read potential header");
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_READ_FAILED;
    }
    
    /* If magic number matches, it's a header file */
    if (header.magic == SAGE_MAGIC_NUMBER) {
        DEBUG_LOG("File has SAGE header (magic: 0x%08X)", header.magic);
        fseek(file, original_pos, SEEK_SET);
        return 0; /* Not headerless */
    }
    
    /* If we got here, there's no valid header. Check for headerless format */
    fseek(file, 0, SEEK_SET);
    
    /* Try to read ntrees and tothalos values */
    read_items = fread(&ntrees, sizeof(int), 1, file);
    if (read_items != 1) {
        IO_ERROR_LOG(IO_ERROR_READ_FAILED, "check_headerless", NULL, 
                   "Failed to read ntrees value");
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_READ_FAILED;
    }
    
    read_items = fread(&tothalos, sizeof(int), 1, file);
    if (read_items != 1) {
        IO_ERROR_LOG(IO_ERROR_READ_FAILED, "check_headerless", NULL, 
                   "Failed to read tothalos value");
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_READ_FAILED;
    }
    
    /* Sanity check the values - if they're both positive and reasonable, 
       this is likely a headerless file */
    if (ntrees > 0 && tothalos > 0 && ntrees <= 1000000 && tothalos <= 100000000) {
        DEBUG_LOG("File appears to be headerless (ntrees: %d, tothalos: %d)", 
                 ntrees, tothalos);
        fseek(file, original_pos, SEEK_SET);
        return 1; /* Headerless */
    }
    
    /* If we got here, the file format is unrecognized */
    IO_ERROR_LOG(IO_ERROR_FORMAT, "check_headerless", NULL, 
               "Unrecognized file format (not a valid SAGE file)");
    fseek(file, original_pos, SEEK_SET);
    return -IO_ERROR_FORMAT;
}

/**
 * @brief   Gets the size of a file
 *
 * @param   file    File pointer to check
 * @return  File size in bytes, or negative error code
 *
 * This function determines the size of a file by seeking to the end
 * and getting the position. It restores the file position afterward.
 */
long get_file_size(FILE* file) {
    long original_pos, file_size;
    
    if (file == NULL) {
        IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "get_file_size", NULL, 
                   "Cannot get size of NULL file pointer");
        return -IO_ERROR_FILE_NOT_FOUND;
    }
    
    /* Save current file position */
    original_pos = ftell(file);
    if (original_pos < 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "get_file_size", NULL, 
                   "Failed to get current file position");
        return -IO_ERROR_SEEK_FAILED;
    }
    
    /* Seek to end of file */
    if (fseek(file, 0, SEEK_END) != 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "get_file_size", NULL, 
                   "Failed to seek to end of file");
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_SEEK_FAILED;
    }
    
    /* Get position (file size) */
    file_size = ftell(file);
    if (file_size < 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "get_file_size", NULL, 
                   "Failed to get end file position");
        fseek(file, original_pos, SEEK_SET);
        return -IO_ERROR_SEEK_FAILED;
    }
    
    /* Restore original position */
    if (fseek(file, original_pos, SEEK_SET) != 0) {
        IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "get_file_size", NULL, 
                   "Failed to restore file position");
        return -IO_ERROR_SEEK_FAILED;
    }
    
    return file_size;
}
