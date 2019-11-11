#include <string.h>
#include <stdlib.h>

#include "hashtable_api.h"
#include "fnv_1a_api.h"


// Size allocated for a new table, in number of entries
#define INITIAL_TABLE_SIZE (64)


// Calculates the size of a single table entry in bytes
#define ENTRY_SIZE_BYTES(table) \
    ((table)->data_size_bytes + sizeof(hashtable_entry_t))


// Gets a pointer to the entry at the provided index
#define INDEX_TABLE(table, index) \
    (hashtable_entry_t *) \
    (((uint8_t *) table->table) + (ENTRY_SIZE_BYTES(table) * (index)))


// If the table load percentage reaches this value or higher, we will resize
#define MAX_TABLE_LOAD_PERCENTAGE   (70)


// Calculate the table load percentage
#define LOAD_PERCENTAGE(table) (((table)->used * 100u) / (table)->size)


/**
 * Enumeration of all possible states that a hashtable entry can be in
 */
typedef enum
{
    ENTRY_STATUS_UNUSED = 0,
    ENTRY_STATUS_USED,
    ENTRY_STATUS_DELETED
} hashtable_entry_status_e;


/**
 * Structure representing a single entry in the hashtable
 */
typedef struct
{
    uint32_t hash;   // Hash of the string key for this item
    uint8_t status;  // Entry status; must be one of hashtable_entry_status_e
    char data[];     // Pointer to data block; allocated by ulist.c
} hashtable_entry_t;


static void _init_new_table(hashtable_t *table)
{
    for (size_t i = 0; i < table->size; i++)
    {
        hashtable_entry_t *entry = INDEX_TABLE(table, i);
        entry->status = (uint8_t) ENTRY_STATUS_UNUSED;
    }
}


static hashtable_entry_t *_find_empty_slot(hashtable_t *table, uint32_t hash)
{
    uint32_t index = hash % table->size;

    // Get the first entry to try
    hashtable_entry_t *entry = INDEX_TABLE(table, index);

    while (ENTRY_STATUS_USED == (hashtable_entry_status_e) entry->status)
    {
        /* Keep going around (linear probing) until we find an unused
         * or deleted entry. */
        index = (index + 1u) % table->size;
        entry = INDEX_TABLE(table, index);
    }

    return entry;
}


static hashtable_entry_t *_find_used_slot(hashtable_t *table, uint32_t hash)
{
    uint32_t index = hash % table->size;

    // Get the first entry to try
    hashtable_entry_t *entry = INDEX_TABLE(table, index);

    while (ENTRY_STATUS_UNUSED != (hashtable_entry_status_e) entry->status)
    {
        // Don't check hash on deleted entries
        if (ENTRY_STATUS_USED == (hashtable_entry_status_e) entry->status)
        {
            // Found the entry we're looking for
            if (entry->hash == hash)
            {
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

    table->table = malloc(ENTRY_SIZE_BYTES(table) * new_size);
    if (NULL == table->table)
    {
        return HASHTABLE_MEMORY_ERROR;
    }

    table->size = new_size;
    table->used = 0u;

    _init_new_table(table);

    /* Insert all entries into new table, re-using the hashes we already
     * calculated to avoid doing that again */
    for (size_t i = 0; i < old_size; i++)
    {
        size_t offset = i * ENTRY_SIZE_BYTES(table);

        hashtable_entry_t *old_entry = (hashtable_entry_t *)
                                       (((uint8_t *) old_table) + offset);

        if (ENTRY_STATUS_USED != (hashtable_entry_status_e) old_entry->status)
        {
            continue;
        }

        hashtable_entry_t *new_entry = _find_empty_slot(table, old_entry->hash);
        (void) memcpy(new_entry, old_entry, ENTRY_SIZE_BYTES(table));
        table->used += 1u;
    }

    free(old_table);
    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_create(hashtable_t *table, size_t data_size_bytes)
{
    memset(table, 0, sizeof(hashtable_t));
    table->data_size_bytes = data_size_bytes;

    table->table = malloc(ENTRY_SIZE_BYTES(table) * INITIAL_TABLE_SIZE);
    if (NULL == table->table)
    {
        return HASHTABLE_MEMORY_ERROR;
    }

    _init_new_table(table);
    table->size = INITIAL_TABLE_SIZE;
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

    free(table->table);
    memset(table, 0, sizeof(hashtable_t));
    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_put(hashtable_t *table, char *key, void *data,
                                 uint32_t *hash_output)
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
    uint32_t hash = fnv_1a_hash(key, strlen(key));
    hashtable_entry_t *entry = _find_empty_slot(table, hash);

    if (NULL != hash_output)
    {
        *hash_output = hash;
    }

    // Populate entry
    (void) memcpy(entry->data, data, table->data_size_bytes);

    entry->hash = hash;
    entry->status = (uint8_t) ENTRY_STATUS_USED;
    table->used += 1u;

    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_get(hashtable_t *table, char *key, void **data_ptr)
{
    if ((NULL == table) || (NULL == key) || (NULL == data_ptr))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    uint32_t hash = fnv_1a_hash(key, strlen(key));

    hashtable_entry_t *entry = _find_used_slot(table, hash);
    if (NULL == entry)
    {
        return HASHTABLE_NO_ITEM;
    }

    *data_ptr = entry->data;
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

    uint32_t hash = fnv_1a_hash(key, strlen(key));

    hashtable_entry_t *entry = _find_used_slot(table, hash);
    if (NULL == entry)
    {
        return HASHTABLE_NO_ITEM;
    }

    entry->status = (uint8_t) ENTRY_STATUS_DELETED;
    return HASHTABLE_OK;
}
