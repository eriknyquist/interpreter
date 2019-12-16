#include <stdint.h>
#include "hashtables_api.h"

uint8_t _pointer_comparison_func(char *str1, char *str2)
{
    return (str1 == str2);
}

hashtable_status_e create_string_comparison_hashtable(hashtable_t *table,
                                                      size_t data_size_bytes)
{
    hashtable_config_t cfg;

    cfg.data_size_bytes = data_size_bytes;
    cfg.hash_func = NULL;
    cfg.strcmp_func = NULL;

    return hashtable_create(table, &cfg);
}


hashtable_status_e create_pointer_comparison_hashtable(hashtable_t *table,
                                                       size_t data_size_bytes)
{
    hashtable_config_t cfg;

    cfg.data_size_bytes = data_size_bytes;
    cfg.hash_func = NULL;
    cfg.strcmp_func = _pointer_comparison_func;

    return hashtable_create(table, &cfg);
}
