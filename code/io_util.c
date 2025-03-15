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

/* Buffer management system - declare all functions before use */
#define MAX_BUFFERED_FILES 16
static struct {
  FILE *file;
  IOBuffer *buffer;
} BufferedFiles[MAX_BUFFERED_FILES] = {0};

/* Buffer management system prototypes defined in header as non-static */

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
  return ((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8);
}

/**
 * @brief   Swaps the bytes in a 32-bit unsigned integer
 *
 * @param   value    The value to swap
 * @return  The byte-swapped value
 */
uint32_t swap_uint32(uint32_t value) {
  return ((value & 0x000000FF) << 24) | ((value & 0x0000FF00) << 8) |
         ((value & 0x00FF0000) >> 8) | ((value & 0xFF000000) >> 24);
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
         ((value & 0x00000000FF000000ULL) << 8) |
         ((value & 0x000000FF00000000ULL) >> 8) |
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
 * @param   file_endian   The endianness of the file (SAGE_LITTLE_ENDIAN or
 * SAGE_BIG_ENDIAN)
 * @return  1 if same endianness, 0 if different
 */
int is_same_endian(int file_endian) { return file_endian == SAGE_HOST_ENDIAN; }

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
 * It supports common data sizes (2, 4, and 8 bytes). The data is modified in
 * place.
 */
void *swap_bytes_if_needed(void *data, size_t size, size_t count,
                           int file_endian) {
  /* Return early if no conversion is needed */
  if (is_same_endian(file_endian) || data == NULL || count == 0) {
    return data;
  }

  /* Cast to byte array for easier manipulation */
  unsigned char *bytes = (unsigned char *)data;
  unsigned char tmp;
  size_t i, j;

  /* Swap bytes based on element size */
  switch (size) {
  case 2: /* 16-bit values */
    for (i = 0; i < count; i++) {
      tmp = bytes[i * 2];
      bytes[i * 2] = bytes[i * 2 + 1];
      bytes[i * 2 + 1] = tmp;
    }
    break;

  case 4: /* 32-bit values */
    for (i = 0; i < count; i++) {
      for (j = 0; j < 2; j++) {
        tmp = bytes[i * 4 + j];
        bytes[i * 4 + j] = bytes[i * 4 + 3 - j];
        bytes[i * 4 + 3 - j] = tmp;
      }
    }
    break;

  case 8: /* 64-bit values */
    for (i = 0; i < count; i++) {
      for (j = 0; j < 4; j++) {
        tmp = bytes[i * 8 + j];
        bytes[i * 8 + j] = bytes[i * 8 + 7 - j];
        bytes[i * 8 + 7 - j] = tmp;
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
int write_sage_header(FILE *file, int endianness) {
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
int read_sage_header(FILE *file, struct SAGEFileHeader *header) {
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
    IO_WARNING_LOG(
        IO_ERROR_INVALID_HEADER, "read_header", NULL,
        "Invalid file header magic number: 0x%08X (expected: 0x%08X)",
        header->magic, SAGE_MAGIC_NUMBER);
    return IO_ERROR_INVALID_HEADER;
  }

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
int check_file_compatibility(const struct SAGEFileHeader *header) {
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
 * @brief   Gets the size of a file
 *
 * @param   file    File pointer to check
 * @return  File size in bytes, or negative error code
 *
 * This function determines the size of a file by seeking to the end
 * and getting the position. It restores the file position afterward.
 */
long get_file_size(FILE *file) {
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

/**
 * @brief   Registers a buffer with a file
 *
 * @param   file    File to register
 * @param   buffer  Buffer to associate with the file
 * @return  0 on success, non-zero on error
 *
 * This function registers a buffer with a file in the buffer management system.
 */
int register_buffer(FILE *file, IOBuffer *buffer) {
  int i;

  if (file == NULL || buffer == NULL) {
    return IO_ERROR_BUFFER;
  }

  /* Find an empty slot or an existing entry for this file */
  for (i = 0; i < MAX_BUFFERED_FILES; i++) {
    if (BufferedFiles[i].file == NULL || BufferedFiles[i].file == file) {
      BufferedFiles[i].file = file;
      BufferedFiles[i].buffer = buffer;
      return 0;
    }
  }

  /* No empty slots found */
  IO_ERROR_LOG(IO_ERROR_BUFFER, "register_buffer", NULL,
               "Maximum number of buffered files (%d) exceeded",
               MAX_BUFFERED_FILES);
  return IO_ERROR_BUFFER;
}

/**
 * @brief   Retrieves a buffer for a file
 *
 * @param   file    File to get buffer for
 * @return  Associated buffer, or NULL if not found
 *
 * This function looks up a file in the buffer management system
 * and returns its associated buffer.
 */
IOBuffer *get_buffer(FILE *file) {
  int i;

  if (file == NULL) {
    return NULL;
  }

  /* Find the file in the registry */
  for (i = 0; i < MAX_BUFFERED_FILES; i++) {
    if (BufferedFiles[i].file == file) {
      return BufferedFiles[i].buffer;
    }
  }

  /* File not found in registry */
  return NULL;
}

/**
 * @brief   Unregisters a buffer
 *
 * @param   file    File to unregister
 * @return  0 on success, non-zero on error
 *
 * This function removes a file from the buffer management system.
 */
int unregister_buffer(FILE *file) {
  int i;

  if (file == NULL) {
    return IO_ERROR_BUFFER;
  }

  /* Find the file in the registry */
  for (i = 0; i < MAX_BUFFERED_FILES; i++) {
    if (BufferedFiles[i].file == file) {
      BufferedFiles[i].file = NULL;
      BufferedFiles[i].buffer = NULL;
      return 0;
    }
  }

  /* File not found in registry */
  return IO_ERROR_BUFFER;
}

/**
 * @brief   Determines the optimal buffer size for a file
 *
 * @param   file    File pointer to analyze
 * @return  Recommended buffer size in bytes
 *
 * This function analyzes the file size to determine an optimal buffer
 * size for I/O operations. Larger files get larger buffers to improve
 * throughput, while smaller files get smaller buffers to reduce memory usage.
 */
size_t get_optimal_buffer_size(FILE *file) {
  long file_size;

  if (file == NULL) {
    WARNING_LOG("Cannot determine optimal buffer size for NULL file");
    return IO_BUFFER_MEDIUM; /* Use default for invalid files */
  }

  file_size = get_file_size(file);
  if (file_size < 0) {
    WARNING_LOG("Failed to get file size for buffer size determination");
    return IO_BUFFER_MEDIUM; /* Use default if file size unknown */
  }

  /* Choose buffer size based on file size */
  if (file_size < 100 * 1024) { /* < 100 KB */
    return IO_BUFFER_SMALL;
  } else if (file_size < 10 * 1024 * 1024) { /* < 10 MB */
    return IO_BUFFER_MEDIUM;
  } else if (file_size < 100 * 1024 * 1024) { /* < 100 MB */
    return IO_BUFFER_LARGE;
  } else { /* >= 100 MB */
    return IO_BUFFER_XLARGE;
  }
}

/**
 * @brief   Creates a new I/O buffer
 *
 * @param   size    Size of the buffer in bytes
 * @param   mode    Buffer mode (read, write, or both)
 * @param   file    File to associate with the buffer
 * @return  Pointer to the new buffer, or NULL on error
 *
 * This function creates a new buffer of the specified size for I/O operations.
 * The buffer is associated with the given file and mode.
 */
IOBuffer *create_buffer(size_t size, IOBufferMode mode, FILE *file) {
  IOBuffer *buffer;

  if (file == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "create_buffer", NULL,
                 "Cannot create buffer for NULL file");
    return NULL;
  }

  if (size == 0) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "create_buffer", NULL,
                 "Cannot create buffer of size 0");
    return NULL;
  }

  /* Allocate the buffer structure */
  buffer = (IOBuffer *)malloc(sizeof(IOBuffer));
  if (buffer == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "create_buffer", NULL,
                 "Failed to allocate IOBuffer structure");
    return NULL;
  }

  /* Allocate the buffer memory */
  buffer->buffer = malloc(size);
  if (buffer->buffer == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "create_buffer", NULL,
                 "Failed to allocate buffer memory (%zu bytes)", size);
    free(buffer);
    return NULL;
  }

  /* Initialize the buffer */
  buffer->size = size;
  buffer->position = 0;
  buffer->valid_size = 0;
  buffer->dirty = 0;
  buffer->file = file;
  buffer->mode = mode;
  buffer->file_offset = ftell(file); /* Record the current file position */

  return buffer;
}

/**
 * @brief   Frees an I/O buffer
 *
 * @param   buffer  Buffer to free
 *
 * This function frees the memory used by an I/O buffer.
 * If the buffer is dirty (contains unsaved changes), it
 * flushes the buffer to disk before freeing.
 */
void free_buffer(IOBuffer *buffer) {
  if (buffer == NULL) {
    return;
  }

  /* Flush any pending writes */
  if (buffer->dirty) {
    flush_buffer(buffer);
  }

  /* Free the buffer memory */
  if (buffer->buffer != NULL) {
    free(buffer->buffer);
    buffer->buffer = NULL;
  }

  /* Free the buffer structure */
  free(buffer);
}

/**
 * @brief   Flushes a buffer to disk - SIMPLIFIED IMPLEMENTATION
 *
 * @param   buffer  Buffer to flush
 * @return  0 on success, non-zero on error
 *
 * This function provides a simplified and robust way to flush buffer
 * contents to disk. It handles position tracking properly and ensures
 * data integrity.
 */
int flush_buffer(IOBuffer *buffer) {
  size_t written;
  long current_position, new_position;

  /* Validate parameters */
  if (buffer == NULL || buffer->buffer == NULL) {
    return IO_ERROR_BUFFER;
  }

  /* Only flush if there's data to flush */
  if (!buffer->dirty || buffer->valid_size == 0) {
    return 0; /* Nothing to flush */
  }

  /* Save current file position */
  current_position = ftell(buffer->file);
  if (current_position < 0) {
    return IO_ERROR_SEEK_FAILED;
  }

  /* Go to buffer's starting position in the file */
  if (fseek(buffer->file, buffer->file_offset, SEEK_SET) != 0) {
    return IO_ERROR_SEEK_FAILED;
  }

  /* Write the buffer contents */
  written = fwrite(buffer->buffer, 1, buffer->valid_size, buffer->file);

  /* Check if write was successful */
  if (written != buffer->valid_size) {
    /* Try to restore file position */
    fseek(buffer->file, current_position, SEEK_SET);
    return IO_ERROR_WRITE_FAILED;
  }

  /* Force data to be written to OS buffers */
  fflush(buffer->file);

  /* Get new file position after writing */
  new_position = ftell(buffer->file);

  /* Update buffer tracking information */
  buffer->file_offset = new_position;
  buffer->position = 0;
  buffer->valid_size = 0;
  buffer->dirty = 0;

  /* Restore original file position */
  if (fseek(buffer->file, current_position, SEEK_SET) != 0) {
    return IO_ERROR_SEEK_FAILED;
  }

  return 0;
}

/**
 * @brief   Fills a buffer with data from its file
 *
 * @param   buffer     Buffer to fill
 * @param   min_fill   Minimum number of bytes to fill
 * @return  Number of bytes filled, or negative error code
 *
 * This function reads data from the file into the buffer.
 * It only has an effect for read and read-write buffers.
 */
int fill_buffer(IOBuffer *buffer, size_t min_fill) {
  size_t read_size, bytes_read;
  long current_pos;

  if (buffer == NULL || buffer->buffer == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "fill_buffer", NULL,
                 "Cannot fill NULL buffer");
    return -IO_ERROR_BUFFER;
  }

  /* Only fill if buffer is in read or read-write mode */
  if (buffer->mode == IO_BUFFER_WRITE) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "fill_buffer", NULL,
                 "Cannot fill write-only buffer");
    return -IO_ERROR_BUFFER;
  }

  /* Get current file position */
  current_pos = ftell(buffer->file);
  if (current_pos < 0) {
    IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "fill_buffer", NULL,
                 "Failed to get current file position");
    return -IO_ERROR_SEEK_FAILED;
  }

  /* Calculate how much to read */
  read_size = buffer->size;
  if (min_fill > 0 && min_fill < read_size) {
    read_size = min_fill;
  }

  /* Read data into buffer */
  bytes_read = fread(buffer->buffer, 1, read_size, buffer->file);

  /* Handle end of file */
  if (bytes_read < read_size && feof(buffer->file)) {
    clearerr(buffer->file); /* Clear EOF indicator */
  }

  /* Update buffer state */
  buffer->position = 0;
  buffer->valid_size = bytes_read;
  buffer->dirty = 0;

  return bytes_read;
}

/**
 * @brief   Reads data from a buffered file
 *
 * @param   buffer   I/O buffer to read from
 * @param   data     Memory location to read into
 * @param   size     Size of each element
 * @param   count    Number of elements to read
 * @return  Number of elements successfully read
 *
 * This function reads data from a buffered file. It attempts to use
 * data already in the buffer, refilling as needed. If the requested
 * data exceeds the buffer size, it reads directly from the file.
 */
size_t buffered_read(IOBuffer *buffer, void *data, size_t size, size_t count) {
  size_t total_size, bytes_read, elements_read, remaining;
  size_t buffer_remaining, copy_size;
  char *dest;
  long file_pos;

  if (buffer == NULL || buffer->buffer == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_read", NULL,
                 "Cannot read from NULL buffer");
    return 0;
  }

  if (data == NULL) {
    IO_ERROR_LOG(IO_ERROR_READ_FAILED, "buffered_read", NULL,
                 "Cannot read into NULL pointer");
    return 0;
  }

  if (size == 0 || count == 0) {
    return 0; /* Nothing to read */
  }

  /* Check if buffer is in read mode */
  if (buffer->mode == IO_BUFFER_WRITE) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_read", NULL,
                 "Cannot read from write-only buffer");
    return 0;
  }

  total_size = size * count;
  dest = (char *)data;
  elements_read = 0;
  remaining = total_size;

  /* If the request is larger than the buffer, read directly from file */
  if (total_size > buffer->size) {
    /* Flush buffer if dirty */
    if (buffer->dirty) {
      if (flush_buffer(buffer) != 0) {
        IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_read", NULL,
                     "Failed to flush buffer before large read");
        return 0;
      }
    }

    /* Get current file position */
    file_pos = ftell(buffer->file);
    if (file_pos < 0) {
      IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "buffered_read", NULL,
                   "Failed to get file position for direct read");
      return 0;
    }

    /* Read directly from file */
    bytes_read = fread(data, 1, total_size, buffer->file);
    elements_read = bytes_read / size;

    /* Update buffer position to match file position */
    buffer->position = 0;
    buffer->valid_size = 0;

    return elements_read;
  }

  /* Try to satisfy read from buffer */
  while (remaining > 0) {
    /* Check if we need to fill the buffer */
    buffer_remaining = buffer->valid_size - buffer->position;

    if (buffer_remaining == 0) {
      /* Buffer is empty, refill it */
      if (fill_buffer(buffer, remaining) <= 0) {
        /* End of file or error */
        break;
      }
      buffer_remaining = buffer->valid_size;
    }

    /* Copy data from buffer to destination */
    copy_size = (buffer_remaining < remaining) ? buffer_remaining : remaining;
    memcpy(dest, (char *)buffer->buffer + buffer->position, copy_size);

    /* Update positions */
    buffer->position += copy_size;
    dest += copy_size;
    remaining -= copy_size;
  }

  /* Calculate how many elements were read */
  bytes_read = total_size - remaining;
  elements_read = bytes_read / size;

  return elements_read;
}

/**
 * @brief   Writes data to a buffered file - SIMPLIFIED IMPLEMENTATION
 *
 * @param   buffer   I/O buffer to write to
 * @param   data     Data to write
 * @param   size     Size of each element
 * @param   count    Number of elements to write
 * @return  Number of elements successfully written
 *
 * This function provides a simplified and robust implementation of buffered
 * writing. It accumulates small writes in the buffer and flushes when needed,
 * while handling large writes directly.
 */
size_t buffered_write(IOBuffer *buffer, const void *data, size_t size,
                      size_t count) {
  size_t total_size, bytes_written, elements_written;

  /* Validate parameters */
  if (buffer == NULL || buffer->buffer == NULL || data == NULL) {
    return 0; /* Invalid parameters */
  }

  if (size == 0 || count == 0) {
    return 0; /* Nothing to write */
  }

  /* Check write permission */
  if (buffer->mode == IO_BUFFER_READ) {
    return 0; /* Cannot write to read-only buffer */
  }

  /* Calculate total data size */
  total_size = size * count;

  /* APPROACH 1: For large writes, bypass buffer and write directly */
  if (total_size > buffer->size / 2) {
    /* First flush any existing buffered data */
    if (buffer->dirty) {
      /* Save current position */
      long current_pos = ftell(buffer->file);

      /* Seek to buffer start position for writing */
      fseek(buffer->file, buffer->file_offset, SEEK_SET);

      /* Write buffer contents */
      fwrite(buffer->buffer, 1, buffer->valid_size, buffer->file);

      /* Mark buffer as clean */
      buffer->dirty = 0;

      /* Update buffer position tracking */
      buffer->file_offset = ftell(buffer->file);
      buffer->position = 0;
      buffer->valid_size = 0;

      /* Restore file position */
      fseek(buffer->file, current_pos, SEEK_SET);
    }

    /* Now write the data directly */
    bytes_written = fwrite(data, 1, total_size, buffer->file);
    elements_written = bytes_written / size;

    /* Update buffer state to match new file position */
    buffer->file_offset = ftell(buffer->file);
    buffer->position = 0;
    buffer->valid_size = 0;

    return elements_written;
  }

  /* APPROACH 2: For small writes, use buffer */

  /* Check if buffer needs to be flushed before writing */
  if (buffer->position + total_size > buffer->size) {
    /* Save current position */
    long current_pos = ftell(buffer->file);

    /* Seek to buffer start position for writing */
    fseek(buffer->file, buffer->file_offset, SEEK_SET);

    /* Write buffer contents */
    fwrite(buffer->buffer, 1, buffer->valid_size, buffer->file);

    /* Update buffer state */
    buffer->dirty = 0;
    buffer->file_offset = ftell(buffer->file);
    buffer->position = 0;
    buffer->valid_size = 0;

    /* Restore file position */
    fseek(buffer->file, current_pos, SEEK_SET);
  }

  /* Copy data to buffer */
  memcpy((char *)buffer->buffer + buffer->position, data, total_size);

  /* Update buffer state */
  buffer->position += total_size;
  buffer->valid_size = buffer->position;
  buffer->dirty = 1;

  /* Successfully buffered all elements */
  return count;
}

/**
 * @brief   Seeks to a position in a buffered file
 *
 * @param   buffer   I/O buffer to seek in
 * @param   offset   Offset from the reference position
 * @param   whence   Reference position (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return  0 on success, non-zero on error
 *
 * This function changes the current position in a buffered file.
 * If the new position is outside the buffer, it flushes any changes
 * and resets the buffer state. If the position is within the buffer,
 * it simply adjusts the buffer position.
 *
 * The function maintains proper synchronization between buffer position
 * and file position, updating the file_offset field to ensure consistency.
 */
int buffered_seek(IOBuffer *buffer, long offset, int whence) {
  long absolute_offset, buffer_start, buffer_end;
  long current_file_pos;

  if (buffer == NULL || buffer->buffer == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_seek", NULL,
                 "Cannot seek in NULL buffer");
    return IO_ERROR_BUFFER;
  }

  /* Get current file position */
  current_file_pos = ftell(buffer->file);
  if (current_file_pos < 0) {
    IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "buffered_seek", NULL,
                 "Failed to get current file position");
    return IO_ERROR_SEEK_FAILED;
  }

  /* Calculate the absolute offset in the file */
  switch (whence) {
  case SEEK_SET:
    absolute_offset = offset;
    break;

  case SEEK_CUR:
    /* For SEEK_CUR, we need to adjust for the buffer position */
    absolute_offset = current_file_pos + offset;
    break;

  case SEEK_END: {
    /* Need to get file size for SEEK_END */
    long file_size = get_file_size(buffer->file);
    if (file_size < 0) {
      return -file_size; /* Error already logged */
    }
    absolute_offset = file_size + offset;
    break;
  }

  default:
    IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "buffered_seek", NULL,
                 "Invalid whence value: %d", whence);
    return IO_ERROR_SEEK_FAILED;
  }

  /* Calculate buffer boundaries in file coordinates */
  buffer_start = buffer->file_offset;
  buffer_end = buffer_start + buffer->valid_size;

  /* Check if new position is within buffer */
  if (absolute_offset >= buffer_start && absolute_offset < buffer_end) {
    /* New position is within buffer, adjust buffer position */
    buffer->position = absolute_offset - buffer_start;

    /* Update file position to match */
    if (fseek(buffer->file, absolute_offset, SEEK_SET) != 0) {
      IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "buffered_seek", NULL,
                   "Failed to seek to position %ld within buffer",
                   absolute_offset);
      return IO_ERROR_SEEK_FAILED;
    }

    return 0;
  }

  /* New position is outside buffer, need to flush and reset */
  if (buffer->dirty) {
    if (flush_buffer(buffer) != 0) {
      IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_seek", NULL,
                   "Failed to flush buffer during seek");
      return IO_ERROR_BUFFER;
    }
  }

  /* Perform seek in the underlying file */
  if (fseek(buffer->file, absolute_offset, SEEK_SET) != 0) {
    IO_ERROR_LOG(IO_ERROR_SEEK_FAILED, "buffered_seek", NULL,
                 "Failed to seek to position %ld", absolute_offset);
    return IO_ERROR_SEEK_FAILED;
  }

  /* Reset buffer state and update file_offset */
  buffer->position = 0;
  buffer->valid_size = 0;
  buffer->dirty = 0;
  buffer->file_offset = absolute_offset;

  return 0;
}

/**
 * @brief   Opens a file with buffering
 *
 * @param   filename     Name of the file to open
 * @param   mode         File mode string ("r", "w", etc.)
 * @param   buffer_size  Size of the buffer to use (0 for automatic)
 * @return  File pointer on success, NULL on error
 *
 * This function opens a file with an associated I/O buffer.
 * It creates and attaches a buffer structure that is used for
 * subsequent I/O operations. The buffer is automatically sized
 * if buffer_size is 0.
 */
FILE *buffered_fopen(const char *filename, const char *mode,
                     size_t buffer_size) {
  FILE *file;
  IOBuffer *buffer;
  IOBufferMode buffer_mode;

  if (filename == NULL || mode == NULL) {
    IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "buffered_fopen", NULL,
                 "Invalid filename or mode");
    return NULL;
  }

  /* Open the file */
  file = fopen(filename, mode);
  if (file == NULL) {
    IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "buffered_fopen", filename,
                 "Failed to open file with mode '%s'", mode);
    return NULL;
  }

  /* Determine buffer mode from file mode */
  if (strchr(mode, '+') != NULL) {
    buffer_mode = IO_BUFFER_READWRITE;
  } else if (strchr(mode, 'r') != NULL) {
    buffer_mode = IO_BUFFER_READ;
  } else {
    buffer_mode = IO_BUFFER_WRITE;
  }

  /* Determine buffer size if not specified */
  if (buffer_size == 0) {
    buffer_size = get_optimal_buffer_size(file);
  }

  /* Create buffer and associate with file */
  buffer = create_buffer(buffer_size, buffer_mode, file);
  if (buffer == NULL) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_fopen", filename,
                 "Failed to create buffer for file");
    fclose(file);
    return NULL;
  }

  /* Register buffer with file */
  if (register_buffer(file, buffer) != 0) {
    IO_ERROR_LOG(IO_ERROR_BUFFER, "buffered_fopen", filename,
                 "Failed to register buffer for file");
    free_buffer(buffer);
    fclose(file);
    return NULL;
  }

  return file;
}

/**
 * @brief   Explicitly flushes a buffered file
 *
 * @param   file    File pointer to flush
 * @return  0 on success, non-zero on error
 *
 * This function flushes any pending changes to disk but keeps
 * the file and buffer open for further operations. It also ensures
 * that the system file buffer is flushed to disk.
 */
int buffered_flush(FILE *file) {
  IOBuffer *buffer;
  int result = 0;

  if (file == NULL) {
    IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "buffered_flush", NULL,
                 "Cannot flush NULL file pointer");
    return EOF;
  }

  /* Retrieve the buffer associated with this file */
  buffer = get_buffer(file);

  /* If buffer exists, flush it */
  if (buffer != NULL && buffer->dirty) {
    result = flush_buffer(buffer);
  }

  /* Also flush the system buffers for this file */
  if (result == 0) {
    if (fflush(file) != 0) {
      IO_ERROR_LOG(IO_ERROR_WRITE_FAILED, "buffered_flush", NULL,
                   "Failed to flush system file buffers");
      result = IO_ERROR_WRITE_FAILED;
    }
  }

  return result;
}

/**
 * @brief   Closes a buffered file
 *
 * @param   file    File pointer to close
 * @return  0 on success, EOF on error
 *
 * This function flushes any pending changes to the file,
 * frees the associated buffer, and closes the file.
 * It ensures all data is properly written to disk before closing.
 */
int buffered_fclose(FILE *file) {
  IOBuffer *buffer;
  int result;

  if (file == NULL) {
    IO_ERROR_LOG(IO_ERROR_FILE_NOT_FOUND, "buffered_fclose", NULL,
                 "Cannot close NULL file pointer");
    return EOF;
  }

  /* Retrieve the buffer associated with this file */
  buffer = get_buffer(file);

  /* If buffer exists, force flush it */
  if (buffer != NULL) {
    /* Force a flush even if not marked dirty, just to be safe */
    if (buffer->valid_size > 0) {

      /* Explicitly write buffer content at correct offset */
      if (fseek(file, buffer->file_offset, SEEK_SET) != 0) {
        WARNING_LOG(
            "Failed to seek to buffer start during close, data may be lost");
      } else {
        size_t written = fwrite(buffer->buffer, 1, buffer->valid_size, file);
        if (written != buffer->valid_size) {
          WARNING_LOG(
              "Failed to write buffer during close: %zu of %zu bytes written",
              written, buffer->valid_size);
        }
      }
    }

    /* Flush any pending system buffers */
    fflush(file);

    /* Free buffer and unregister */
    free(buffer->buffer);
    free(buffer);
    unregister_buffer(file);
  }

  /* Close the file */
  result = fclose(file);
  if (result != 0) {
    IO_ERROR_LOG(IO_ERROR_CLOSE_FAILED, "buffered_fclose", NULL,
                 "Failed to close file");
  } else {
  }

  return result;
}
