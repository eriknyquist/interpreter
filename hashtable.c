#include <string.h>

#include "hashtable_api.h"
#include "ulist_api.h"

#define MAX_STRING_SIZE (256)

#define CHECK_ULIST_ERR(func) {        \
    ulist_status_e _err_code = func;   \
    if (ULIST_ERROR_MEM == _err_code)  \
    {                                  \
        return HASHTABLE_MEMORY_ERROR; \
    }                                  \
    else if (ULIST_OK != _err_code)    \
    {                                  \
        return HASHTABLE_ERROR;        \
    }                                  \
}                                      \


static hashtable_status_e calculate_hash(char *string, uint32_t *hash)
{
    uint32_t calculated_hash = 7u;

    for (uint32_t i = 0; string[i]; i++)
    {
        if (i == MAX_STRING_SIZE)
        {
            return HASHTABLE_KEY_TOO_LONG;
        }

        calculated_hash += ((uint8_t)string[i])  * i * 31u;
    }

    *hash = calculated_hash;
    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_create(hashtable_t *table, size_t data_size_bytes)
{
    memset(table, 0, sizeof(hashtable_t));
    table->data_size_bytes = data_size_bytes;

    // TODO: make table resizeable
    table->size = NUM_TABLE_SLOTS;

    return HASHTABLE_OK;
}


/**
 * @see hashtable_api.h
 */
hashtable_status_e hashtable_destroy(hashtable_t *table)
{
    for (uint32_t i = 0; i < NUM_TABLE_SLOTS; i++)
    {
        ulist_t *slot = &table->table[i];
        if (0u < slot->item_size_bytes)
        {
            CHECK_ULIST_ERR(ulist_destroy(slot));
        }
    }

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

    uint32_t hash;
    hashtable_status_e hash_err = calculate_hash(key, &hash);
    if (HASHTABLE_OK != hash_err)
    {
        return hash_err;
    }

    ulist_t *slot = &table->table[hash % NUM_TABLE_SLOTS];
    table->collisions = 0u;

    if (0u == slot->item_size_bytes)
    {
        // Initialize list instance if not yet used
        size_t entry_size = sizeof(hashtable_entry_t) + table->data_size_bytes;
        CHECK_ULIST_ERR(ulist_create(slot, entry_size, LIST_ITEMS_PER_NODE));
    }
    else
    {
        // Verify this string key hasn't already been used
        hashtable_entry_t *curr;
        ulist_status_e err = ULIST_OK;
        table->collisions = slot->num_items;

        while (1)
        {
            err = ulist_get_next_item(slot, (void **)&curr);
            if (ULIST_END == err)
            {
                break;
            }

            if (ULIST_OK != err)
            {
                return HASHTABLE_ERROR;
            }

            if (hash == curr->hash)
            {
                return HASHTABLE_HASH_ALREADY_EXISTS;
            }
        }
    }

    hashtable_entry_t *allocd_entry;

    // fetch a chunk of memory from ulist
    CHECK_ULIST_ERR(ulist_alloc(slot, slot->num_items, (void **)&allocd_entry));

    // Copy data
    (void) memcpy(allocd_entry->data, data, table->data_size_bytes);

    allocd_entry->hash = hash;
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

    uint32_t hash;
    hashtable_status_e hash_err = calculate_hash(key, &hash);
    if (HASHTABLE_OK != hash_err)
    {
        return hash_err;
    }

    ulist_t *slot = &table->table[hash % NUM_TABLE_SLOTS];

    if (0u == slot->item_size_bytes)
    {
        return HASHTABLE_NO_ITEM;
    }

    CHECK_ULIST_ERR(ulist_set_iteration_start_index(slot, 0u));

    hashtable_entry_t *curr;
    ulist_status_e err = ULIST_OK;

    while (ULIST_END != err)
    {
        err = ulist_get_next_item(slot, (void **)&curr);
        if (ULIST_OK != err)
        {
            return HASHTABLE_ERROR;
        }

        if (hash == curr->hash)
        {
            *data_ptr = curr->data;
            return HASHTABLE_OK;
        }
    }

    return HASHTABLE_NO_ITEM;
}
