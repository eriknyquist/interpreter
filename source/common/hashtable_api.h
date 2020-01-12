#ifndef _HASHTABLE_API_H
#define _HASHTABLE_API_H

#include <stdint.h>
#include "ulist_api.h"

/* -- Beginning of tunable settings section -- */

// Size allocated for a new table, in number of entries
#define INITIAL_TABLE_SIZE (64)

// If the table load percentage reaches this value or higher, we will resize
#define MAX_TABLE_LOAD_PERCENTAGE   (70)

/* -- End of tunable settings section -- */


/**
 * Status codes returned by hashtable functions
 */
typedef enum
{
    HASHTABLE_OK,                  // Operation completed successfully
    HASHTABLE_LAST_ENTRY,          // Returned by hashtable_next on last entry
    HASHTABLE_NO_ITEM,             // No item matching the provided key
    HASHTABLE_KEY_TOO_LONG,        // Max. key length of 256 bytes exceeded
    HASHTABLE_KEY_ALREADY_EXISTS,  // Item with the provided key already exists
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
 * Structure representing runtime information about a hashtable_t instance
 */
typedef struct
{
    size_t entry_count;              // Number of entries in the hashtable
    size_t size_bytes;               // Total size allocated for table in bytes
    unsigned load_factor_percent;    // Load factor as a percentage (0 == empty)
} hashtable_stats_t;


/**
 * Structure representing a single entry in the hashtable
 */
typedef struct
{
    char *key;                  // String key
    uint32_t hash;              // Hash of string key
    uint8_t status;             // Entry status; must be one of hashtable_entry_status_e
    char data[];                // Pointer to data section
} hashtable_entry_t;


/**
 * Structure representing a hashtable
 */
typedef struct
{
    size_t data_size_bytes;              // Data size of a single table entry
    hashtable_hash_func_t hash_func;     // Hash generation function
    hashtable_strcmp_func_t strcmp_func; // String comparison function
    hashtable_entry_t *last_written;     // Last entry written with hashtable_put
    size_t size;                         // Total number of slots in the table
    size_t used;                         // Number of slots used in the table
    size_t index;                        // Entry index used by hashtable_next
    void *table;                         // Pointer to table data
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
 * Add an entry to a hashtable. Note that the pointer to the string key ('key')
 * is copied as-is into the hashtable entry, so the caller should ensure that
 * space for the string key remains allocated while the hashtable is in use.
 *
 * @param table  Pointer to hashtable instance
 * @param key    Pointer to NULL-terminated string key used to access the entry.
 * @param data   Pointer to data for hashtable entry.
 *
 * @return       HASHTABLE_OK if successful, #hastable_status_e otherwise
 */
hashtable_status_e hashtable_put(hashtable_t *table, char *key, void *data);


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
 * Gets the next entry in the hashtable. Allows for each entry in the hashtable
 * to be retrieved, by calling this function until HASHTABLE_LAST_ENTRY is
 * returned. When HASHTABLE_LAST_ENTRY is returned, the entry provided is the
 * last entry in the provided table, and the next call to hashtable_next will
 * wrap back around to the first entry. Note that entries are returned in the
 * order that they occur in memory reserved for the hashtable (so, unrelated to
 * the order in which they were added to the table).
 *
 * @param   table     Pointer to hashtable_t instance to get next entry from
 * @param   data_ptr  Pointer to location to store pointer to next entry
 *
 * @return  HASHTABLE_OK (or HASHTABLE_LAST_ENTRY if provided entry is the last
 *          one) if next entry was fetched successfully.
 */
hashtable_status_e hashtable_next(hashtable_t *table, void **data_ptr);


/**
 * Return usage information about the given hashtable (see hashtable_stats_t
 * struct definition)
 *
 * @param   table   Pointer to hashtable_t instance to return information about
 * @param   stats   Pointer to hashtable_stats_t structure to populate
 *
 * @return  HASHTABLE_OK if usage information was gathered successfully
 */
hashtable_status_e hashtable_stats(hashtable_t *table, hashtable_stats_t *stats);


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
