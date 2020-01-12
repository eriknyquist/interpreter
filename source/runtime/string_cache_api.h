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
 * Structure representing a string cache
 */
typedef struct
{
    hashtable_t string_table;
} string_cache_t;


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
 * Initialize a string cache. Allocates some space for initial entries.
 *
 * @param   cache   Pointer to string_cache_t structure to intialize
 *
 * @return  STRING_CACHE_OK if string cache was initialized successfully
 */
string_cache_status_e string_cache_create(string_cache_t *cache);


/**
 * Destroy a string cache. Frees any memory allocated for storing cached strings.
 *
 * @param   cache   Pointer to string cache to destroy
 *
 * @return  STRING_CACHE_OK if string cache was destroyed successfully
 */
string_cache_status_e string_cache_destroy(string_cache_t *cache);


/**
 * Adds a string to the string cache. If the given string does not already exist
 * in the cache, it will be converted to a byte_string_t and added to the cache.
 * If the string does already exist in the string cache, then a pointer to the
 * cached byte_string_t will be returned, and no new byte_string_t object will
 * be created.
 *
 * @param   cache          Pointer to string cache to add string to
 * @param   string_to_add  Pointer to NULL terminated string to add to cache
 * @param   cached_string  Pointer to location to store pointer to cached byte_string_t
 *
 * @return  STRING_CACHE_OK if string was added string cache successfully, or
 *          STRING_CACHE_ALREADY_CACHED if provided string was already in the cache
 */
string_cache_status_e string_cache_add(string_cache_t *cache,
                                       char *string_to_add,
                                       byte_string_t **cached_string);


/**
 * Fetch some usage information about the provided string_cache_t instance
 * (see string_cache_stats_t struct definition).
 *
 * @param   cache   Pointer to string cache to get usage information for
 * @param   stats   Pointer to string_cache_stats_t struct to populate
 *
 * @return  STRING_CACHE_OK if usage information was fetched successfully
 */
string_cache_status_e string_cache_stats(string_cache_t *cache,
                                         string_cache_stats_t *stats);

#endif /* STRING_CACHE_API_H */
