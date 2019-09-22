#include <stdio.h>
#include <string.h>
#include "hashtable_api.h"

void set_value(char *name, int value)
{
    hashtable_entry_t entry;

    entry.value = value;
    (void)strncpy(entry.string, name, sizeof(entry.string));
    (void)hashtable_put(&entry);
}

void print_value(char *name)
{
    hashtable_entry_t *entry;
    if (HASHTABLE_NO_ITEM == hashtable_get(name, &entry))
    {
        printf("No item matching '%s'\n", name);
        return;
    }

    printf("%s: %d\n", entry->string, entry->value);
}

int main(void)
{
    hashtable_init();

    set_value("value1", 4);
    set_value("value2", 5);
    set_value("value3", 6);
    set_value("my_thing", 7);

    print_value("value1");
    print_value("value2");
    print_value("value3");
    print_value("my_thing");
    print_value("myd_thing");

    hashtable_destroy();
}
