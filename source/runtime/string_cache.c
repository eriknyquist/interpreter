#include <string.h>
#include <stdint.h>
#include "string_cache_api.h"

#include <stdio.h>

/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_create(string_cache_t *cache)
{
    if (NULL == cache)
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    hashtable_status_e err = create_string_comparison_hashtable(&cache->string_table,
                                                                sizeof(byte_string_t));
    if (HASHTABLE_MEMORY_ERROR == err)
    {
        return STRING_CACHE_MEMORY_ERROR;
    }
    else if (HASHTABLE_OK != err)
    {
        return STRING_CACHE_ERROR;
    }

    return STRING_CACHE_OK;
}


/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_destroy(string_cache_t *cache)
{
    if (NULL == cache)
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    hashtable_status_e err = hashtable_destroy(&cache->string_table);
    if (HASHTABLE_OK != err)
    {
        return STRING_CACHE_ERROR;
    }

    return STRING_CACHE_OK;
}


/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_stats(string_cache_t *cache,
                                         string_cache_stats_t *stats)
{
    if ((NULL == cache) || (NULL == stats))
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    hashtable_stats_t table_stats;

    if (HASHTABLE_OK != hashtable_stats(&cache->string_table, &table_stats))
    {
        return STRING_CACHE_ERROR;
    }

    stats->string_count = table_stats.entry_count;
    stats->table_size_bytes = table_stats.size_bytes;

    // Initialize total string bytes count
    stats->total_string_bytes = 0u;

    hashtable_status_e err;
    byte_string_t *string;

    // Iterate over all entries in the string cache to get the total byte count
    do {
        err = hashtable_next(&cache->string_table, (void **) &string);
        if ((HASHTABLE_OK != err) && (HASHTABLE_LAST_ENTRY != err))
        {
            return STRING_CACHE_ERROR;
        }

        stats->total_string_bytes += string->total_bytes;
    }
    while (HASHTABLE_LAST_ENTRY != err);

    return STRING_CACHE_OK;
}


/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_add(string_cache_t *cache,
                                       char *string_to_add,
                                       byte_string_t **cached_string)
{
    if ((NULL == cache) || (NULL == string_to_add))
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    string_cache_status_e ret = STRING_CACHE_ALREADY_CACHED;
    byte_string_t *cached;
    hashtable_status_e err;

    err = hashtable_get(&cache->string_table, string_to_add,
                        (void **) &cached);

    if (HASHTABLE_NO_ITEM == err)
    {
        byte_string_t byte_string;

        // String does not already exist, create new byte string object
        if (BYTE_STRING_OK != byte_string_create(&byte_string))
        {
            return STRING_CACHE_ERROR;
        }

        if (BYTE_STRING_OK != byte_string_add_bytes(&byte_string,
                                                    strlen(string_to_add),
                                                    string_to_add))
        {
            return STRING_CACHE_ERROR;
        }

        // Add byte string structure to hashtable, use string contents as key
        err = hashtable_put(&cache->string_table, byte_string.bytes,
                            &byte_string);

        if (HASHTABLE_OK != err)
        {
            return STRING_CACHE_ERROR;
        }

        // Provide pointer to newly allocated string to the caller
        cached = (byte_string_t *) cache->string_table.last_written->data;
        ret = STRING_CACHE_OK;
    }
    else if (HASHTABLE_OK != err)
    {
        ret = STRING_CACHE_ERROR;
    }

    if (NULL != cached_string)
    {
        *cached_string = cached;
    }

    return ret;
}
