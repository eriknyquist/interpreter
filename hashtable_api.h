#ifndef _HASHTABLE_API_H
#define _HASHTABLE_API_H

#include <stdint.h>
#include "ulist_api.h"

#define NUM_TABLE_SLOTS (128)
#define MAX_STRING_SIZE (128)
#define LIST_ITEMS_PER_NODE (10u)


/**
 * Status codes returned by hashtable functions
 */
typedef enum
{
    HASHTABLE_OK,
    HASHTABLE_NO_ITEM,
    HASHTABLE_ITEM_ALREADY_EXISTS,
    HASHTABLE_INVALID_PARAM,
    HASHTABLE_MEMORY_ERROR,
    HASHTABLE_ERROR
} hashtable_status_e;

/**
 * Structure representing a single entry in the hashtable
 */
typedef struct
{
    char key[MAX_STRING_SIZE];   // String key used to access this item
    char data[];                 // Pointer to data block; allocated by ulist.c
} hashtable_entry_t;

/**
 * Structure representing a hashtable
 */
typedef struct
{
    ulist_t table[NUM_TABLE_SLOTS]; // Array of unrolled linked lists
    size_t data_size_bytes;         // Data size of a single table entry
    uint32_t collisions;            // Collision counter (reset for each 'put')
} hashtable_t;

hashtable_status_e hashtable_create(hashtable_t *table, size_t item_size_bytes);

hashtable_status_e hashtable_destroy(hashtable_t *table);

hashtable_status_e hashtable_put(hashtable_t *table, char *string, void *data);

hashtable_status_e hashtable_get(hashtable_t *table, char *string, void **data_ptr);


#endif /* _HASHTABLE_API_H */
