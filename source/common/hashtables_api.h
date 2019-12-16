#ifndef HASHTABLES_API_H
#define HASHTABLES_API_H

#include "hashtable_api.h"


hashtable_status_e create_string_comparison_hashtable(hashtable_t *table,
                                                      size_t data_size_bytes);


hashtable_status_e create_pointer_comparison_hashtable(hashtable_t *table,
                                                       size_t data_size_bytes);


#endif /* HASHTABLES_API_H */
