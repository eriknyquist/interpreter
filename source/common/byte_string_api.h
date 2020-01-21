#ifndef BYTE_STRING_API_H_
#define BYTE_STRING_API_H_


#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>


/* Enumeration of all status codes returned by byte_string_api functions */
typedef enum
{
    BYTE_STRING_OK,
    BYTE_STRING_INVALID_PARAM,
    BYTE_STRING_INDEX_OUT_OF_RANGE,
    BYTE_STRING_MEMORY_ERROR,
    BYTE_STRING_OUTPUT_TRUNCATED,
    BYTE_STRING_ERROR
} byte_string_status_e;


/* Structure representing a dynamically sized contiguous chunk of bytes */
typedef struct
{
    size_t size;
    char *bytes;
} byte_string_t;


/**
 * Initialize a new byte string instance
 *
 * @param    string         Pointer to byte string structure to intialize
 * @param    size           Size to allocate for string in bytes
 * @param    initial_bytes  If non-NULL, 'size' bytes will be copied from this
 *                          pointer to the newly created byte string
 *
 * @return   BYTE_STRING_OK if byte string was initialized successfully
 */
byte_string_status_e byte_string_create(byte_string_t *string, size_t size,
                                        char *initial_bytes);


/**
 * Destroy an initialized byte string instance, and free any memory that may
 * have been allocated
 *
 * @param    string    Pointer to byte string structure to destroy
 *
 * @return   BYTE_STRING_OK if byte string was destroyed successfully
 */
byte_string_status_e byte_string_destroy(byte_string_t *string);


/**
 * Print a formatted string to byte string, starting from the first byte of the
 * byte string
 *
 * @param    string     Pointer to byte string structure
 * @param    size       Max. number of bytes to write
 * @param    format     Format string
 * @param    ...        Format arguments
 *
 * @return   BYTE_STRING_OK if printing was successful
 */
byte_string_status_e byte_string_snprintf(byte_string_t *string, size_t size,
                                          const char *format, ...);


#endif /* BYTE_STRING_API_H */
