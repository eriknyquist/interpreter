#include <string.h>
#include <stdint.h>
#include "string_cache_api.h"


string_cache_status_e string_cache_create(string_cache_t *cache)
{
    if (NULL == cache)
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    hashtable_config_t cfg;

    cfg.data_size_bytes = sizeof(byte_string_t);
    cfg.hash_func = NULL;
    cfg.strcmp_func = NULL;

    hashtable_status_e err = hashtable_create(&cache->string_table, &cfg);
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

string_cache_status_e string_cache_add(string_cache_t *cache,
                                       char *string_to_add,
                                       char **cached_string)
{
    if ((NULL == cache) || (NULL == string_to_add))
    {
        return STRING_CACHE_INVALID_PARAM;
    }

    byte_string_t *existing_string;
    hashtable_status_e err;
    char *cached;

    err = hashtable_get(&cache->string_table, string_to_add,
                        (void **) &existing_string);

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
        cached = byte_string.bytes;
    }
    else if (HASHTABLE_OK == err)
    {
        // Item already exists, provide pointer to the caller
        cached = existing_string->bytes;
    }
    else
    {
        return STRING_CACHE_ERROR;
    }

    if (NULL != cached_string)
    {
        *cached_string = cached;
    }

    return STRING_CACHE_OK;
}
