#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "hashtable_api.h"


#define CHAR_LOWER_BOUND (0x20) // Start of printable ASCII chars
#define CHAR_UPPER_BOUND (0x7e) // End of printable ASCII chars


#define MIN_STRING_SIZE (8)
#define MAX_STRING_SIZE (32)


#define NUM_ENTRIES_TO_TEST (10000)


typedef struct
{
    char key[MAX_STRING_SIZE + 1];
    int data;
} test_data_t;


static test_data_t test_hashtable_entries[NUM_ENTRIES_TO_TEST];


static void _populate_test_data(test_data_t *entry)
{
    int size = MIN_STRING_SIZE + (rand() % (MAX_STRING_SIZE - MIN_STRING_SIZE));

    for (int i = 0; i < size; i++)
    {
        char c = CHAR_LOWER_BOUND + (rand() % (CHAR_UPPER_BOUND - CHAR_LOWER_BOUND));
        entry->key[i] = c;
    }

    entry->key[size] = '\0';
    entry->data = rand();
}

static int _run_test(void)
{
    hashtable_t hashtable;
    hashtable_status_e err;

    // Populate all test entries with new random data
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        test_data_t *entry = test_hashtable_entries + i;
        _populate_test_data(entry);
    }

    err = hashtable_create(&hashtable, sizeof(int));
    if (HASHTABLE_OK != err)
    {
        printf("hashtable_create failed, status %d\n", err);
        return 1;
    }

    // Put all the entries into the hashtable
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        test_data_t *entry = test_hashtable_entries + i;
        err = hashtable_put(&hashtable, entry->key, (void **) &entry->data, NULL);
        if (HASHTABLE_OK != err)
        {
            printf("hashtable_put failed, status %d\n", err);
            return 1;
        }
    }

    // Pull out all the entries and make sure the data matches expected
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        int *data;
        test_data_t *entry = test_hashtable_entries + i;

        err = hashtable_get(&hashtable, entry->key, (void **) &data);
        if (HASHTABLE_OK != err)
        {
            printf("hashtable_get failed, status %d\n", err);
            return 1;
        }

        if (*data != entry->data)
        {
            printf("expected value %d for key %s, got %d",
                   entry->data, entry->key, *data);
            return 1;
        }

        printf("Successfully verified %s : %d\n", entry->key, entry->data);
    }

    err = hashtable_destroy(&hashtable);
    if (HASHTABLE_OK != err)
    {
        printf("hashtable_destroy failed, status %d\n", err);
        return 1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    srand((unsigned) time(NULL));
    printf("\n%s\n", _run_test() ? "Failure occurred" : "OK");
}
