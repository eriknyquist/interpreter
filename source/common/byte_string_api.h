#ifndef BYTE_STRING_API_H_
#define BYTE_STRING_API_H_


#include <stdint.h>


/* Enumeration of all status codes returned by byte_string_api functions */
typedef enum
{
    BYTE_STRING_OK,
    BYTE_STRING_INVALID_PARAM,
    BYTE_STRING_INDEX_OUT_OF_RANGE,
    BYTE_STRING_MEMORY_ERROR,
    BYTE_STRING_ERROR
} byte_string_status_e;


/* Structure representing a dynamically sized contiguous chunk of bytes */
typedef struct
{
    size_t total_bytes;
    size_t used_bytes;
    uint8_t *bytes;
} byte_string_t;


byte_string_status_e byte_string_destroy(byte_string_t *string);


byte_string_status_e byte_string_create(byte_string_t *string);


byte_string_status_e byte_string_add_bytes(byte_string_t *string, uint8_t *bytes,
                                           size_t num_bytes);


#endif /* BYTE_STRING_API_H */
