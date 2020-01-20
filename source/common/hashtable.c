#include <string.h>
#include <stdlib.h>

#include "memory_manager_api.h"
#include "hashtable_api.h"
#include "fnv_1a_api.h"


// Calculates the size of a single table entry in bytes
#define ENTRY_SIZE_BYTES(table) \
    ((table)->data_size_bytes + sizeof(hashtable_entry_t))

// Gets a pointer to the entry at the provided index
#define INDEX_TABLE(table, index) \
    (hashtable_entry_t *) \
    (((uint8_t *) table->table) + (ENTRY_SIZE_BYTES(table) * (index)))

// Calculate the table load percentage
#define LOAD_PERCENTAGE(table) (((table)->used * 100u) / (table)->size)


/**
 * Enumeration of all possible states that a hashtable entry can be in
 */
typedef enum
{
    /* We're depending on 0 meaning UNUSED, so that we can just memset the whole
     * table to 0 when initializing a new table */
    ENTRY_STATUS_UNUSED = 0,
    ENTRY_STATUS_USED,
    ENTRY_STATUS_DELETED
} entry_status_e;


static uint8_t _default_strcmp_func(char * str1, char * str2)
{
    for (int i = 0; str1[i]; i++)
    {
        if (str1[i] != str2[i])
        {
            return 0;
        }
    }

    return 1;
}


static void _init_new_table(hashtable_t *table)
{
    table->last_written = NULL;
    table->index = 0u;
    table->used = 0u;
    memset(table->table, 0, ENTRY_SIZE_BYTES(table) * table->size);
}


static hashtable_entry_t *_find_empty_slot(hashtable_t *table, char *key,
                                           uint32_t hash)
{
    uint32_t index = hash % table->size;

    // Get the first entry to try
    hashtable_entry_t *entry = INDEX_TABLE(table, index);

    while (ENTRY_STATUS_USED == (entry_status_e) entry->status)
    {
        if (table->strcmp_func(key, entry->key))
        {
            return NULL;
        }

        /* Keep going around (linear probing) until we find an unused
         * or deleted entry. */
        index = (index + 1u) % table->size;
        entry = INDEX_TABLE(table, index);
    }

    return entry;
}


static hashtable_entry_t *_find_used_slot(hashtable_t *table, char *key, uint32_t hash)
{
    uint32_t index = hash % table->size;

    // Get the first entry to try
    hashtable_entry_t *entry = INDEX_TABLE(table, index);

    while (ENTRY_STATUS_UNUSED != (entry_status_e) entry->status)
    {
        // Don't check hash on deleted entries
        if (ENTRY_STATUS_USED == (entry_status_e) entry->status)

        {
            if (table->strcmp_func(key, entry->key))
            {
                // Keys match
                return entry;
            }
        }

        index = (index + 1u) % table->size;
        entry = INDEX_TABLE(table, index);
    }

    return NULL;
}


static hashtable_status_e _resize_table(hashtable_t *table, size_t new_size)
{
    if (table->used > new_size)
    {
        return HASHTABLE_MEMORY_ERROR;
    }

    void *old_table = table->table;
    size_t old_size = table->size;

    table->table = memory_manager_alloc(ENTRY_SIZE_BYTES(table) * new_size);
    if (NULL == table->table)
    {
        return HASHTABLE_MEMORY_ERROR;
    }

    table->size = new_size;

    _init_new_table(table);

    /* Insert all entries into new table, re-using the hashes we already
     * calculated to avoid doing that again */
    for (size_t i = 0; i < old_size; i++)
    {
        size_t offset = i * ENTRY_SIZE_BYTES(table);

        hashtable_entry_t *old_entry = (hashtable_entry_t *)
                                       (((uint8_t *) old_table) + offset);

        if (ENTRY_STATUS_USED != (entry_status_e) old_entry->status)
        {
            continue;
        }

        hashtable_entry_t *new_entry = _find_empty_slot(table, old_entry->key,
                                                        old_entry->hash);
        if (NULL == new_entry)
        {
            // Shouldn't be any duplicate keys in old table
            return HASHTABLE_ERROR;
        }

        (void) memcpy(new_entry, old_entry, ENTRY_SIZE_BYTES(table));
        table->used += 1u;
    }

    memory_manager_free(old_table);
    return HASHTABLE_OK;
}


/* Starting from table->index, find the next used entry in the table and
 * return a pointer to it */
static hashtable_entry_t *_find_next_used_entry(hashtable_t *table)
{
    for (; table->index < table->size; table->index++)
    {
        hashtable_entry_t *entry = INDEX_TABLE(table, table->index);
        if (ENTRY_STATUS_USED == entry->status)
        {
            return entry;
        }
    }

    return NULL;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_create(hashtable_t *table, hashtable_config_t *cfg)
{
    if ((NULL == table) || (NULL == cfg) || (0u == cfg->data_size_bytes))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    // Initialize table structure
    memset(table, 0, sizeof(hashtable_t));
    table->data_size_bytes = cfg->data_size_bytes;

    // Populate hash function
    if (NULL == cfg->hash_func)
    {
        table->hash_func = fnv_1a_32_hash;
    }
    else
    {
        table->hash_func = cfg->hash_func;
    }

    // Populate string comparison function
    if (NULL == cfg->strcmp_func)
    {
        table->strcmp_func = _default_strcmp_func;
    }
    else
    {
        table->strcmp_func = cfg->strcmp_func;
    }

    // Allocate some initial space
    table->table = memory_manager_alloc(ENTRY_SIZE_BYTES(table) * INITIAL_TABLE_SIZE);
    if (NULL == table->table)
    {
        return HASHTABLE_MEMORY_ERROR;
    }

    table->size = INITIAL_TABLE_SIZE;
    _init_new_table(table);

    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_destroy(hashtable_t *table)
{
    if (NULL == table->table)
    {
        return HASHTABLE_OK;
    }

    memory_manager_free(table->table);
    memset(table, 0, sizeof(hashtable_t));
    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_put(hashtable_t *table, char *key, void *data)
{
    if ((NULL == table) || (NULL == key) || (NULL == data))
    {
        return HASHTABLE_INVALID_PARAM;
    }


    // Check table load factor, resize if needed
    if (MAX_TABLE_LOAD_PERCENTAGE <= LOAD_PERCENTAGE(table))
    {
        size_t new_size = table->size * 2u;
        hashtable_status_e err = _resize_table(table, new_size);
        if (HASHTABLE_OK != err)
        {
            return err;

        }
    }

    // Calculate hash and find corresponding entry
    uint32_t hash = table->hash_func(key, strlen(key));
    table->last_written = _find_empty_slot(table, key, hash);
    if (NULL == table->last_written)
    {
        return HASHTABLE_KEY_ALREADY_EXISTS;
    }

    // Populate entry
    (void) memcpy(table->last_written->data, data, table->data_size_bytes);


    table->last_written->key = key;
    table->last_written->hash = hash;
    table->last_written->status = (uint8_t) ENTRY_STATUS_USED;
    table->used += 1u;

    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_get(hashtable_t *table, char *key, void **data_ptr)
{
    if ((NULL == table) || (NULL == key))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    uint32_t hash = table->hash_func(key, strlen(key));

    hashtable_entry_t *entry = _find_used_slot(table, key, hash);
    if (NULL == entry)
    {
        return HASHTABLE_NO_ITEM;
    }

    if (NULL != data_ptr)
    {
        *data_ptr = entry->data;
    }

    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_next(hashtable_t *table, void **data_ptr)
{
    if ((NULL == table) || (0u == table->used))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    hashtable_status_e ret;

    // Get the entry we're writing to data_ptr for this call
    hashtable_entry_t *entry = _find_next_used_entry(table);

    if (NULL != data_ptr)
    {
        *data_ptr = entry->data;
    }


    // Find next entry to prime for next call
    table->index += 1u;
    (void) _find_next_used_entry(table);

    if (table->index == table->size)
    {
        ret = HASHTABLE_LAST_ENTRY;
        table->index = 0u;
    }
    else
    {
        ret = HASHTABLE_OK;
    }

    return ret;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_stats(hashtable_t *table, hashtable_stats_t *stats)
{
    if ((NULL == table) || (NULL == stats))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    stats->entry_count = table->used;
    stats->size_bytes = ENTRY_SIZE_BYTES(table) * table->size;
    stats->load_factor_percent = LOAD_PERCENTAGE(table);

    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_delete(hashtable_t *table, char *key)
{
    if ((NULL == table) || (NULL == key))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    uint32_t hash = table->hash_func(key, strlen(key));

    hashtable_entry_t *entry = _find_used_slot(table, key, hash);
    if (NULL == entry)
    {
        return HASHTABLE_NO_ITEM;
    }

    entry->status = (uint8_t) ENTRY_STATUS_DELETED;
    return HASHTABLE_OK;
}
