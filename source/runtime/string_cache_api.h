#ifndef STRING_CACHE_API_H
#define STRING_CACHE_API_H

#include "hashtable_api.h"
#include "byte_string_api.h"


/**
 * Status codes returned by string_cache functions
 */
typedef enum
{
    STRING_CACHE_OK,
    STRING_CACHE_INVALID_PARAM,
    STRING_CACHE_MEMORY_ERROR,
    STRING_CACHE_ERROR
} string_cache_status_e;


/**
 * Structure representing a string cache
 */
typedef struct
{
    hashtable_t string_table;
} string_cache_t;


string_cache_status_e string_cache_create(string_cache_t *cache);


string_cache_status_e string_cache_add(string_cache_t *cache,
                                       char *string_to_add,
                                       char **cached_string);

#endif /* STRING_CACHE_API_H */
