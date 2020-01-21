#include <stdlib.h>
#include <string.h>

#include "memory_manager_api.h"
#include "byte_string_api.h"


/**
 * @see byte_string_api.h
 */
byte_string_status_e byte_string_create(byte_string_t *string, size_t size,
                                        char *initial_bytes)
{
    if (NULL == string)
    {
        return BYTE_STRING_INVALID_PARAM;
    }

    string->size = size;
    string->bytes = memory_manager_alloc(size);

    if (NULL == string->bytes)
    {
        return BYTE_STRING_MEMORY_ERROR;
    }

    if (NULL != initial_bytes)
    {
        memcpy(string->bytes, initial_bytes, size);
    }

    return BYTE_STRING_OK;
}


/**
 * @see byte_string_api.h
 */
byte_string_status_e byte_string_snprintf(byte_string_t *string, size_t size,
                                          const char *format, ...)
{
    if ((NULL == string) || (NULL == format))
    {
        return BYTE_STRING_INVALID_PARAM;
    }

    va_list va_args;

    va_start(va_args, format);

    if (vsnprintf((char *) string->bytes, size, format, va_args) >= size)
    {
        return BYTE_STRING_OUTPUT_TRUNCATED;
    }

    va_end(va_args);

    return BYTE_STRING_OK;
}


/**
 * @see byte_string_api.h
 */
byte_string_status_e byte_string_destroy(byte_string_t *string)
{
    if (NULL == string)
    {
        return BYTE_STRING_INVALID_PARAM;
    }

    if (NULL != string->bytes)
    {
        memory_manager_free(string->bytes);
    }

    memset(string, 0, sizeof(byte_string_t));

    return BYTE_STRING_OK;
}
