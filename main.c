#include <stdio.h>
#include <string.h>
#include "hashtable_api.h"

void set_value(hashtable_t *table, char *name, int value)
{
    hashtable_status_e err = hashtable_put(table, name, &value);
    printf("put %s:%d (err=%d, collisions=%d)\n", name, value, err, table->collisions);
}

void print_value(hashtable_t *table, char *name)
{
    int *value;
    if (HASHTABLE_NO_ITEM == hashtable_get(table, name, (void **)&value))
    {
        printf("No item matching '%s'\n", name);
        return;
    }

    printf("%s: %d\n", name, *value);
}

int main(void)
{
    hashtable_t table;

    hashtable_create(&table, sizeof(int));

    set_value(&table, "value1", 4);
    set_value(&table, "value2", 5);
    set_value(&table, "value3", 6);
    set_value(&table, "my_thing", 7);

    print_value(&table, "value1");
    print_value(&table, "value2");
    print_value(&table, "value3");
    print_value(&table, "my_thing");
    print_value(&table, "myd_thing");

    hashtable_destroy(&table);
}
