#ifndef _HASHTABLE_API_H
#define _HASHTABLE_API_H

#include <stdint.h>

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


hashtable_status_e hashtable_init(void);

hashtable_status_e hashtable_destroy(void);

hashtable_status_e hashtable_put(hashtable_entry_t *entry);

hashtable_status_e hashtable_get(char *string, hashtable_entry_t **entry);


#endif /* _HASHTABLE_API_H */
