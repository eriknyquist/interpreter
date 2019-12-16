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

/* Hashtable hash function signature */
typedef uint32_t (*hashtable_hash_func_t)(void *, size_t);

/* Hashtable string comparison function signature */
typedef uint8_t (*hashtable_strcmp_func_t)(char *, char *);

/**
 * Structure representing configurable parameters for a hashtable
 */
typedef struct
{
    /* Size in bytes of a single hashtable entry. The data pointer passed to
     * hashtable_put and hashtable_get are expected to point to objects of
     * this size. */
    size_t data_size_bytes;

    /* Function for producing a 32-bit hash of a byte string. If NULL,
     * fnv_1a_32_hash will be used. */
    hashtable_hash_func_t hash_func;

    /* Function for comparing two NULL-terminated string keys. Should return 1
     * if strings match, and 0 if they do not match. If NULL, the equivalent of
     * the standard strcmp function will be used. */
    hashtable_strcmp_func_t strcmp_func;
} hashtable_config_t;

/**
 * Structure representing a hashtable
 */
typedef struct
{
    size_t data_size_bytes;              // Data size of a single table entry
    hashtable_hash_func_t hash_func;     // Hash generation function
    hashtable_strcmp_func_t strcmp_func; // String comparison function
    size_t size;                         // Total number of slots in the table
    size_t used;                         // Number of slots used in the table
    void *table;
} hashtable_t;


/**
 * Initialize a hashtable instance.
 *
 * @param table       Pointer to hashtable instance
 * @param cfg         Pointer to a hashtable_config_t structure
 *
 * @return            HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_create(hashtable_t *table, hashtable_config_t *cfg);


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
