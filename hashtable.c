#include <string.h>

#include "hashtable_api.h"
#include "ulist_api.h"


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

ulist_t _hashtable[NUM_TABLE_SLOTS];


uint32_t calculate_hash(char *string)
{
    uint32_t hash = 7u;

    for (uint32_t i = 0; string[i]; i++)
    {
        hash += ((uint8_t)string[i]) * 31u;
    }

    return hash;
}

hashtable_status_e hashtable_init(void)
{
    memset(_hashtable, 0, sizeof(ulist_t) * NUM_TABLE_SLOTS);
    return HASHTABLE_OK;
}

hashtable_status_e hashtable_destroy(void)
{
    for (uint32_t i = 0; i < NUM_TABLE_SLOTS; i++)
    {
        ulist_t *slot = &_hashtable[i];
        if (0u < slot->item_size_bytes)
        {
            CHECK_ULIST_ERR(ulist_destroy(slot));
        }
    }

    return HASHTABLE_OK;
}

hashtable_status_e hashtable_put(hashtable_entry_t *entry)
{
    if (NULL == entry)
    {
        return HASHTABLE_INVALID_PARAM;
    }

    uint32_t index = calculate_hash(entry->string) % NUM_TABLE_SLOTS;
    ulist_t *slot = &_hashtable[index];

    if (0u == slot->item_size_bytes)
    {
        // Initialize list instance if not yet used
        CHECK_ULIST_ERR(ulist_create(slot, sizeof(hashtable_entry_t),
                                     LIST_ITEMS_PER_NODE));
    }
    else
    {
        // Verify this string key hasn't already been used
        hashtable_entry_t *curr;
        ulist_status_e err = ULIST_OK;

        while (ULIST_END != err)
        {
            err = ulist_get_next_item(slot, (void **)&curr);
            if (ULIST_OK != err)
            {
                return HASHTABLE_ERROR;
            }

            if (0 == strcmp(entry->string, curr->string))
            {
                return HASHTABLE_ITEM_ALREADY_EXISTS;
            }
        }

    }

    CHECK_ULIST_ERR(ulist_append_item(slot, entry));
    return HASHTABLE_OK;
}

hashtable_status_e hashtable_get(char *string, hashtable_entry_t **entry)
{
    if ((NULL == entry) || (NULL == string))
    {
        return HASHTABLE_INVALID_PARAM;
    }

    uint32_t index = calculate_hash(string) % NUM_TABLE_SLOTS;
    ulist_t *slot = &_hashtable[index];

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

        if (0 == strcmp(string, curr->string))
        {
            *entry = curr;
            return HASHTABLE_OK;
        }
    }

    return HASHTABLE_NO_ITEM;
}
