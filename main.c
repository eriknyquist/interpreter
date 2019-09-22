#include <stdio.h>
#include <string.h>
#include "hashtable_api.h"

void set_value(hashtable_t *table, char *name, int value)
{
    hashtable_entry_t entry;

    entry.value = value;
    (void)strncpy(entry.string, name, sizeof(entry.string));
    (void)hashtable_put(table, &entry);
    printf("put %s:%d (collisions=%d)\n", name, value, table->collisions);
}

void print_value(hashtable_t *table, char *name)
{
    hashtable_entry_t *entry;
    if (HASHTABLE_NO_ITEM == hashtable_get(table, name, &entry))
    {
        printf("No item matching '%s'\n", name);
        return;
    }

    printf("%s: %d\n", entry->string, entry->value);
}

int main(void)
{
    hashtable_t table;

    hashtable_create(&table);

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
