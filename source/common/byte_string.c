#include <stdlib.h>
#include <string.h>

#include "byte_string_api.h"


#define BLOCK_SIZE (64)


#define ENLARGE_IF_NEEDED(string, num_bytes)                                  \
    do {                                                                      \
        size_t __needed = string->used_bytes + num_bytes;                     \
        if (string->total_bytes < __needed)                                   \
        {                                                                     \
            byte_string_status_e __err;                                       \
                                                                              \
            __err = _resize_byte_string(string, __needed);                    \
            if (BYTE_STRING_OK != __err)                                      \
            {                                                                 \
                return __err;                                                 \
            }                                                                 \
        }                                                                     \
    }                                                                         \
    while (0);


static byte_string_status_e _resize_byte_string(byte_string_t *string, size_t new_size)
{
    char *temp;

    size_t alloc_size;
    
    alloc_size = (BLOCK_SIZE < new_size) ?
                 BLOCK_SIZE * ((new_size / BLOCK_SIZE) + 1) : BLOCK_SIZE;

    if (NULL == string->bytes)
    {
        // First bytes being added after creation-- need to allocate space
        if ((temp = malloc(alloc_size)) == NULL)
        {
            return BYTE_STRING_MEMORY_ERROR;
        }
    }
    else
    {
        // Space is already allocated, need to realloc

        if ((temp = realloc(string->bytes, alloc_size)) == NULL)
        {
            return BYTE_STRING_MEMORY_ERROR;
        }
    }

    string->bytes = temp;
    string->total_bytes = new_size;

    if (new_size < string->used_bytes)
    {
        string->used_bytes = new_size;
    }

    return BYTE_STRING_OK;
}


byte_string_status_e byte_string_create(byte_string_t *string)
{
    if (NULL == string)
    {
        return BYTE_STRING_INVALID_PARAM;
    }

    memset(string, 0, sizeof(byte_string_t));
    
    return BYTE_STRING_OK;
}


byte_string_status_e byte_string_add_bytes(byte_string_t *string, char *bytes,
                                           size_t num_bytes)
{
    if (NULL == string)
    {
        return BYTE_STRING_INVALID_PARAM;
    }

    ENLARGE_IF_NEEDED(string, num_bytes);

    if ((NULL != bytes) && (0 < num_bytes))
    {
        (void) memcpy(string->bytes + string->used_bytes, bytes, num_bytes);
    }

    return BYTE_STRING_OK;
}


byte_string_status_e byte_string_destroy(byte_string_t *string)
{
    if (NULL == string)
    {
        return BYTE_STRING_INVALID_PARAM;
    }

    if (NULL != string->bytes)
    {
        free(string->bytes);
    }

    memset(string, 0, sizeof(byte_string_t));
    
    return BYTE_STRING_OK;
}
