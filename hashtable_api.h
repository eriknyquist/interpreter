#ifndef _HASHTABLE_API_H
#define _HASHTABLE_API_H

#include <stdint.h>
#include "ulist_api.h"

#define NUM_TABLE_SLOTS (128)
#define MAX_STRING_SIZE (128)
#define LIST_ITEMS_PER_NODE (10u)


typedef enum
{
    HASHTABLE_OK,
    HASHTABLE_NO_ITEM,
    HASHTABLE_ITEM_ALREADY_EXISTS,
    HASHTABLE_INVALID_PARAM,
    HASHTABLE_MEMORY_ERROR,
    HASHTABLE_ERROR
} hashtable_status_e;

typedef struct
{
    char string[MAX_STRING_SIZE];
    int value;
} hashtable_entry_t;

typedef struct
{
    ulist_t table[NUM_TABLE_SLOTS];
    uint32_t collisions;
} hashtable_t;

hashtable_status_e hashtable_create(hashtable_t *table);

hashtable_status_e hashtable_destroy(hashtable_t *table);

hashtable_status_e hashtable_put(hashtable_t *table, hashtable_entry_t *entry);

hashtable_status_e hashtable_get(hashtable_t *table, char *string,
                                 hashtable_entry_t **entry);


#endif /* _HASHTABLE_API_H */
