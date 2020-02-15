#ifndef STRING_CACHE_API_H
#define STRING_CACHE_API_H

#include "hashtables_api.h"
#include "byte_string_api.h"


/**
 * Status codes returned by string_cache functions
 */
typedef enum
{
    STRING_CACHE_OK,
    STRING_CACHE_ALREADY_CACHED,
    STRING_CACHE_INVALID_PARAM,
    STRING_CACHE_MEMORY_ERROR,
    STRING_CACHE_ERROR
} string_cache_status_e;


/**
 * Structure representing runtime information about a string_cache_t
 */
typedef struct
{
    size_t string_count;         // Number of strings in string cache
    size_t table_size_bytes;     // Size of string pointer hashtable in bytes
    size_t total_string_bytes;   // Total memory allocated for string datain bytes
} string_cache_stats_t;


/**
 * Initialize the string cache. Allocates some space for initial entries.
 *
 * @return  STRING_CACHE_OK if string cache was initialized successfully
 */
string_cache_status_e string_cache_init(void);


/**
 * Destroy the string cache. Frees any memory allocated for storing cached strings.
 *
 * @return  STRING_CACHE_OK if string cache was destroyed successfully
 */
string_cache_status_e string_cache_destroy(void);


/**
 * Adds a string to the string cache. If the given string does not already exist
 * in the cache, it will be converted to a byte_string_t and added to the cache.
 * If the string does already exist in the string cache, then a pointer to the
 * cached byte_string_t will be returned, and no new byte_string_t object will
 * be created.
 *
 * @param   string_to_add  Pointer to NULL terminated string to add to cache
 * @param   size           Number of bytes to copy from string_to_add
 * @param   cached_string  Pointer to location to store pointer to cached byte_string_t
 *
 * @return  STRING_CACHE_OK if string was added string cache successfully, or
 *          STRING_CACHE_ALREADY_CACHED if provided string was already in the cache
 */
string_cache_status_e string_cache_add(char *string_to_add,
                                       unsigned size,
                                       byte_string_t **cached_string);


/**
 * Fetch some usage information about the string cache
 * (see string_cache_stats_t struct definition).
 *
 * @param   stats   Pointer to string_cache_stats_t struct to populate
 *
 * @return  STRING_CACHE_OK if usage information was fetched successfully
 */
string_cache_status_e string_cache_stats(string_cache_stats_t *stats);

#endif /* STRING_CACHE_API_H */
