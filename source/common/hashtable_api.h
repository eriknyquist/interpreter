#ifndef _HASHTABLE_API_H
#define _HASHTABLE_API_H

#include <stdint.h>
#include "ulist_api.h"

#define NUM_TABLE_SLOTS (128)
#define LIST_ITEMS_PER_NODE (10u)


/**
 * Status codes returned by hashtable functions
 */
typedef enum
{
    HASHTABLE_OK,
    HASHTABLE_NO_ITEM,             // No item matching the provided key
    HASHTABLE_KEY_TOO_LONG,        // Max. key length of 256 bytes exceeded
    HASHTABLE_HASH_ALREADY_EXISTS, // Item with the provided key already exists
    HASHTABLE_INVALID_PARAM,       // Invalid parameter passed to function
    HASHTABLE_MEMORY_ERROR,        // Memory allocation failed
    HASHTABLE_ERROR                // Unspecified internal error
} hashtable_status_e;


/**
 * Structure representing a hashtable
 */
typedef struct
{
    void *table;
    size_t data_size_bytes;         // Data size of a single table entry
    size_t size;                    // Total number of slots in the table
    size_t used;                    // Number of slots used in the table
    uint32_t collisions;            // Collision counter (reset for each 'put')
} hashtable_t;


/**
 * Initialize a hashtable instance.
 *
 * @param table            Pointer to hashtable instance
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
 * @param table  Pointer to hashtable instance
 *
 * @return       HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_destroy(hashtable_t *table);


/**
 * Add an entry to a hashtable.
 *
 * @param table  Pointer to hashtable instance
 * @param key    Pointer to NULL-terminated string key used to access the entry.
 * @param data   Pointer to data for hashtable entry.
 * @param hash   Optional pointer to write calculated hash to. If not NULL, the
 *               hash calculated for key is written here.
 *
 * @return       HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_put(hashtable_t *table, char *key, void *data,
                                 uint32_t *hash_output);


/**
 * Fetch an entry from a hashtable.
 *
 * @param table     Pointer to hashtable instance
 * @param key       Pointer to NULL-terminated string key for entry to fetch
 * @param data_ptr  Pointer to location to store pointer to fetched entry
 *
 * @return          HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_get(hashtable_t *table, char *key, void **data_ptr);


/**
 * Delete an entry from a hashtable
 *
 * @param table     Pointer to hashtable instance
 * @param key       Pointer to NULL-terminated string key for entry to delete
 *
 * @return          HASHTABLE_OK if successful, #hastable_status_e otherwise
 *
 */
hashtable_status_e hashtable_delete(hashtable_t *table, char *key);


#endif /* _HASHTABLE_API_H */
