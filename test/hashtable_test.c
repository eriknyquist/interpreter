#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "hashtable_api.h"


#define CHAR_LOWER_BOUND (0x20) // Start of printable ASCII chars
#define CHAR_UPPER_BOUND (0x7e) // End of printable ASCII chars


#define MIN_STRING_SIZE (32)
#define MAX_STRING_SIZE (64)

#define NUM_ENTRIES_TO_TEST (1000000)

#define MIN_ENTRIES_TO_DELETE (NUM_ENTRIES_TO_TEST / 100)
#define MAX_ENTRIES_TO_DELETE (NUM_ENTRIES_TO_TEST / 10)


// size of a single entry in bytes, entry struct is not in public hashtable API
#define ENTRY_SIZE (81)

#define RANDRANGE(low, high)  ((low) + (rand() % ((high) - (low))))


#define TIME_HASHTABLE_OP(highvar, lowvar, op, errval)                        \
        uint64_t __start = _timestamp_ms();                                   \
        errval = op;                                                          \
        uint64_t __time = _timestamp_ms() - __start;                          \
                                                                              \
        if ((lowvar == UINT64_MAX) || (__time < lowvar))                      \
        {                                                                     \
            lowvar = __time;                                                  \
        }                                                                     \
                                                                              \
        if ((highvar == UINT64_MAX) || (__time > highvar))                    \
        {                                                                     \
            highvar = __time;                                                 \
        }                                                                     \

#define TIME_HASHTABLE_PUT(op, errval) TIME_HASHTABLE_OP(highest_put_ms, lowest_put_ms, op, errval)
#define TIME_HASHTABLE_GET(op, errval) TIME_HASHTABLE_OP(highest_get_ms, lowest_get_ms, op, errval)


static uint64_t lowest_get_ms = UINT64_MAX;
static uint64_t highest_get_ms = UINT64_MAX;
static uint64_t lowest_put_ms = UINT64_MAX;
static uint64_t highest_put_ms = UINT64_MAX;


static int putcount, getcount, deletecount;

typedef struct
{
    char key[MAX_STRING_SIZE + 1];
    int data;
    uint8_t deleted;
} test_data_t;


static test_data_t test_hashtable_entries[NUM_ENTRIES_TO_TEST];


static uint32_t _timestamp_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}


static void _populate_test_data_entry(int count)
{
    test_data_t *entry = test_hashtable_entries + count;

    int size = RANDRANGE(MIN_STRING_SIZE, MAX_STRING_SIZE);

    for (int i = 0; i < size; i++)
    {
        char c = RANDRANGE(CHAR_LOWER_BOUND, CHAR_UPPER_BOUND);
        entry->key[i] = c;
    }

    entry->key[size] = '\0';

    entry->data = rand();
    entry->deleted = 0u;
}

static int _verify_hashtable_state(hashtable_t *hashtable)
{
    hashtable_status_e err;

    // Pull out all the entries and make sure the data matches expected
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        int *data;
        test_data_t *entry = test_hashtable_entries + i;

        getcount++;
        TIME_HASHTABLE_GET(hashtable_get(hashtable, entry->key, (void **) &data), err);
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
    }

    return 0;
}


static void _generate_test_data(void)
{
    int last_printed = 0;

    // Populate all test entries with random unique string keys
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        _populate_test_data_entry(i);

        int percent = (int) (((float) i) / ((float) NUM_ENTRIES_TO_TEST / 100.0f));
        if ((percent != last_printed) && !(percent % 10))
        {
            last_printed = percent;
            printf("%d%%...\n", percent);
        }
    }

    printf("100%%!\n");
}

static int _run_test(void)
{
    hashtable_t hashtable;
    hashtable_config_t cfg;
    hashtable_status_e err;

    printf("\nGenerating test data...\n\n");
    _generate_test_data();

    cfg.data_size_bytes = sizeof(int);
    cfg.hash_func = NULL;
    cfg.strcmp_func = NULL;

    err = hashtable_create(&hashtable, &cfg);
    if (HASHTABLE_OK != err)
    {
        printf("hashtable_create failed, status %d\n", err);
        return 1;
    }

    printf("\nRunning test...\n");
    // Put all the entries into the hashtable
    for (int i = 0; i < NUM_ENTRIES_TO_TEST; i++)
    {
        test_data_t *entry = test_hashtable_entries + i;

        putcount++;
        TIME_HASHTABLE_PUT(hashtable_put(&hashtable, entry->key, (void **) &entry->data), err);
 
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

        deletecount++;
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
    printf("\n%s\n", _run_test() ? "Failure occurred" : "All OK");

    printf("\n%d gets, %d puts, %d deletes, %d bytes total\n\n", getcount, putcount,
           deletecount, ENTRY_SIZE * NUM_ENTRIES_TO_TEST);

    printf("lowest put time: %lums\n", lowest_put_ms);
    printf("highest put time: %lums\n", highest_put_ms);
    printf("lowest get time: %lums\n", lowest_get_ms);
    printf("highest get time: %lums\n\n", highest_get_ms);
}
