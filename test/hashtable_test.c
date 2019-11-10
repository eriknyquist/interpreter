#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "hashtable_api.h"


#define CHAR_LOWER_BOUND (0x20) // Start of printable ASCII chars
#define CHAR_UPPER_BOUND (0x7e) // End of printable ASCII chars


#define MIN_STRING_SIZE (8)
#define MAX_STRING_SIZE (32)

#define NUM_ENTRIES_TO_TEST (10000)

#define MIN_ENTRIES_TO_DELETE (NUM_ENTRIES_TO_TEST / 100)
#define MAX_ENTRIES_TO_DELETE (NUM_ENTRIES_TO_TEST / 10)


#define RANDRANGE(low, high)  ((low) + (rand() % ((high) - (low))))


typedef struct
{
    char key[MAX_STRING_SIZE + 1];
    int data;
    uint8_t deleted;
} test_data_t;


static test_data_t test_hashtable_entries[NUM_ENTRIES_TO_TEST];


static void _populate_test_data(test_data_t *entry)
{
    int size = RANDRANGE(MIN_STRING_SIZE, MAX_STRING_SIZE);

    for (int i = 0; i < size; i++)
    {
        char c = RANDRANGE(CHAR_LOWER_BOUND, CHAR_UPPER_BOUND);
        entry->key[i] = c;
    }

    entry->key[size] = '\0';
    entry->data = rand();
}

static int _verify_hashtable_state(hashtable_t *hashtable)
{
    hashtable_status_e err;

    // Pull out all the entries and make sure the data matches expected
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        int *data;
        test_data_t *entry = test_hashtable_entries + i;

        if (entry->deleted)
        {
            // Expecting get operation to fail, item is deleted
            if (HASHTABLE_NO_ITEM != err)
            {
                printf("hashtable_get failed, status %d\n", err);
                return 1;
            }

            // Item is deleted, skip value check
            continue;
        }
        else
        {
            // Expecting get operation to succeed
            if (HASHTABLE_OK != err)
            {
                printf("hashtable_get failed, status %d\n", err);
                return 1;
            }
        }

        if (*data != entry->data)
        {
            printf("expected value %d for key %s, got %d",
                   entry->data, entry->key, *data);
            return 1;
        }

        printf("Successfully verified %s : %d\n", entry->key, entry->data);
    }

    return 0;
}


static int _run_test(void)
{
    hashtable_t hashtable;
    hashtable_status_e err;

    // Populate all test entries with new random data, set up expected hashtable state
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        test_data_t *entry = test_hashtable_entries + i;
        _populate_test_data(entry);
        entry->deleted = 0u;
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

    if (_verify_hashtable_state(&hashtable) != 0)
    {
        // Hashtable state didn't match expected after put operations
        return 1;
    }

    // Delete a random number of randomly selected entries
    int entries_to_delete = RANDRANGE(MIN_ENTRIES_TO_DELETE, MAX_ENTRIES_TO_DELETE);
    for (int i = 0; i < entries_to_delete; i++)
    {
        // Select a random entry
        test_data_t *entry = test_hashtable_entries + RANDRANGE(0, NUM_ENTRIES_TO_TEST);

        // Skip entries that we've already deleted
        if (entry->deleted)
        {
            continue;
        }

        err = hashtable_delete(&hashtable, entry->key);
        if (HASHTABLE_OK != err)
        {
            printf("hashtable_delete failed, status %d\n", err);
            return 1;
        }

        entry->deleted = 1u;
    }

    if (_verify_hashtable_state(&hashtable) != 0)
    {
        // Hashtable state didn't match expected after delete operations
        return 1;
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
