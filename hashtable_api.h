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


/**
 * Initialize a hashtable instance.
 *
 * @param table            Pointer to hashtable instance to be initialized
 * @param item_size_bytes  Size of a single entry in the hashtable. The pointer
 *                         passed to hastable_put is expected to point to data
 *                         of this size.
 *
 * @return                 HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_create(hashtable_t *table, size_t item_size_bytes);


/**
 * Destroy a hashtable instance. You should call this on any initialized hashtables
 * when you are done with them, otherwise you may end up with memory leaks.
 *
 * @param table  Pointer to hashtable instance to be destroyed
 *
 * @return       HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_destroy(hashtable_t *table);


/**
 * Add an entry to the hashtable.
 *
 * @param table  Pointer to hashtable instance to be destroyed
 * @param key    String key used to access the entry in the hashtable
 * @param data   Pointer to data for hashtable entry
 *
 * @return       HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_put(hashtable_t *table, char *key, void *data);


/**
 * Fetch an entry from the hashtable.
 *
 * @param table     Pointer to hashtable instance to be destroyed
 * @param key       String key for entry to fetch
 * @param data_ptr  Pointer to location to store pointer to fetched entry
 *
 * @return          HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_get(hashtable_t *table, char *key, void **data_ptr);


#endif /* _HASHTABLE_API_H */
