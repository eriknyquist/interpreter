#include <string.h>
#include <stdint.h>
#include "string_cache_api.h"


static hashtable_t string_table;

/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_init(void)
{
    hashtable_status_e err = create_string_comparison_hashtable(&string_table,
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
string_cache_status_e string_cache_destroy(void)
{
    hashtable_status_e err;

    if (0u < string_table.used)
    {
        do {
            byte_string_t *string;

            err = hashtable_next(&string_table, (void **) &string);
            if ((HASHTABLE_OK != err) && (HASHTABLE_LAST_ENTRY != err))
            {
                return STRING_CACHE_ERROR;
            }

            byte_string_status_e str_err = byte_string_destroy(string);
            if (BYTE_STRING_OK != str_err)
            {
                return STRING_CACHE_ERROR;
            }
        }
        while (HASHTABLE_LAST_ENTRY != err);
    }

    err = hashtable_destroy(&string_table);
    if (HASHTABLE_OK != err)
    {
        return STRING_CACHE_ERROR;
    }

    return STRING_CACHE_OK;
}


/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_stats(string_cache_stats_t *stats)
{
    if (NULL == stats)
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    hashtable_stats_t table_stats;

    if (HASHTABLE_OK != hashtable_stats(&string_table, &table_stats))
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
        err = hashtable_next(&string_table, (void **) &string);
        if ((HASHTABLE_OK != err) && (HASHTABLE_LAST_ENTRY != err))
        {
            return STRING_CACHE_ERROR;
        }

        stats->total_string_bytes += string->size;
    }
    while (HASHTABLE_LAST_ENTRY != err);

    return STRING_CACHE_OK;
}


/**
 * @see string_cache_api.h
 */
string_cache_status_e string_cache_add(char *string_to_add,
                                       unsigned size,
                                       byte_string_t **cached_string)
{
    if (NULL == string_to_add)
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    string_cache_status_e ret = STRING_CACHE_ALREADY_CACHED;
    byte_string_t *cached;
    hashtable_status_e err;

    err = hashtable_get(&string_table, string_to_add, (void **) &cached);

    if (HASHTABLE_NO_ITEM == err)
    {
        byte_string_t byte_string;

        // String does not already exist, create new byte string object
        if (BYTE_STRING_OK != byte_string_create(&byte_string, size + 1, NULL))
        {
            return STRING_CACHE_ERROR;
        }

        memcpy(byte_string.bytes, string_to_add, size);
        byte_string.bytes[size] = '\0'; // NULL-terminate string

        // Add byte string structure to hashtable, use string contents as key
        err = hashtable_put(&string_table, byte_string.bytes, &byte_string);
        if (HASHTABLE_OK != err)
        {
            return STRING_CACHE_ERROR;
        }

        // Provide pointer to newly allocated string to the caller
        cached = (byte_string_t *) string_table.last_written->data;
        ret = STRING_CACHE_OK;
    }
    else if (HASHTABLE_OK != err)
    {
        cached = NULL;
        ret = STRING_CACHE_ERROR;
    }

    if (NULL != cached_string)
    {
        *cached_string = cached;
    }

    return ret;
}
