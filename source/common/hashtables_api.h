/**
 * Helper functions to create hashtables with different configurations
 */

#ifndef HASHTABLES_API_H
#define HASHTABLES_API_H

#include "hashtable_api.h"


/**
 * Create a hashtable that compares string keys by comparing every character
 * of both keys.
 *
 * @param    table            Pointer to hashtable_t structure to initialize
 * @param    data_size_bytes  Size of single hashtable entry (in bytes)
 *
 * @return   Same as hashtable_create
 */
hashtable_status_e create_string_comparison_hashtable(hashtable_t *table,
                                                      size_t data_size_bytes);


/**
 * Create a hashtable that compares string keys by comparing the string data
 * pointers of both keys. When using this type of hashtable, ensure only cached
 * strings are used for string keys when adding new entries (call
 * string_cache_add on the string first).
 *
 * The advantage of this type of of hashtable is that insertions/deletions are
 * faster, since string comparisons only require a single pointer comparison,
 * regardless of the length of either string key. In order for this comparison
 * to be meaningful, however, we need a gaurantee that unique pointers point
 * to unique strings (in other words, that no duplicate strings exist in the
 * pool we draw our string keys from). This is the purpose of the string_cache
 * module.
 *
 * @param    table            Pointer to hashtable_t structure to initialize
 * @param    data_size_bytes  Size of single hashtable entry (in bytes)
 *
 * @return   Same as hashtable_create
 */
hashtable_status_e create_pointer_comparison_hashtable(hashtable_t *table,
                                                       size_t data_size_bytes);


#endif /* HASHTABLES_API_H */
