/*
 * Fixed-size block allocator for small allocations ( < 512 bytes). Heavily
 * influenced by Python's obmalloc.c, although far simpler.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "memory_manager_api.h"


/* Round down size "n" to be a multiple of "a" ("a" must be a power of 2). */
#define ROUND_UP(n, a) (((n) + ((a) - 1)) & ~((a) - 1))

/* Round up size "n" to be a multiple of "a" ("a" must be a power of 2). */
#define ROUND_DOWN(n, a) ((n) & ~((a) - 1))

// Get a pointer to the pool containing the given data pointer
#define GET_POOL_POINTER(heap, data) \
        ((mempool_t *) \
            (((uint8_t *) heap->heap) + \
            ROUND_DOWN(((uint8_t *) data) - heap->heap, POOL_SIZE_BYTES)))


// Maximum usable block offset from the start of a pool
#define MAX_BLOCK_OFFSET (POOL_SIZE_BYTES)

// Maximum usable pool offset from the start of a memheap_t's "heap" member
#define MAX_POOL_OFFSET (HEAP_SIZE_BYTES)


// Calculate number of bytes remaining to be carved off in a pool
#define POOL_BYTES_REMAINING(pool) (POOL_SIZE_BYTES - pool->nextoffset)

// Calculate number of bytes remaining to be carved off in a heap
#define HEAP_BYTES_REMAINING(heap) (HEAP_SIZE_BYTES - heap->nextoffset)


// Number of bytes used at the beginning of a mempool_t struct for housekeeping
#define POOL_OVERHEAD_BYTES ROUND_UP(sizeof(mempool_t), ALIGNMENT_BYTES)


// Checks if a pointer points to an allocated block from the given heap
#define POINTER_IN_HEAP(heap, ptr) ((((uint8_t *) ptr) > heap->heap) && \
                                    (((uint8_t *) ptr) < (heap->heap + HEAP_SIZE_BYTES)))


/* Evaluates to 1 if the given pool is linked in the usedpools table, meaning the
 * pool has already been carved off a heap and has space available for allocations.
 * 0 otherwise */
#define POOL_IN_USE(pool) ((NULL != pool->nextpool) || (NULL != pool->prevpool))


#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define MAX(x, y) (((x) > (y)) ? (x) : (y))


/* Structure representing a pool containing blocks of a single size class.
 *
 * This structure is overlaid at the beginning of a pool, such that the space
 * available for object allocation in a pool is actually
 * POOL_SIZE_BYTES - ROUND_UP(sizeof(mempool_t), ALIGNMENT_BYTES) */
typedef struct mempool
{
    struct mempool *nextpool; // pointer to next pool of this size class
    struct mempool *prevpool; // pointer to previous pool of this size class

    /* offset in bytes of next unused block in this pool, from the
     * beginning of this struct */
    uint16_t nextoffset;

    // size of blocks stored in this pool
    uint16_t block_size;
} mempool_t;


/* Stucture representing a heap, containing a fixed number of pools */
typedef struct memheap
{
    struct memheap *nextheap;
    struct memheap *prevheap;

    /* offset in bytes of next unused pool in this heap, from the
     * beginning of the "heap" member */
    uint32_t nextoffset;

    uint8_t heap[HEAP_SIZE_BYTES];
} memheap_t;


/* Structure representing a doubly-linked list of mempool_t structures */
typedef struct
{
    mempool_t *head;
    mempool_t *tail;
} mempool_list_t;


struct memheap *headheap = NULL;
struct memheap *tailheap = NULL;


/* Table of doubly-linked lists of pools for each size class. Pools linked in
 * this table have blocks available to be allocated, and pools in each list
 * will be sorted in descending order of fullness percentage */
static mempool_list_t usedpools[NUM_SIZE_CLASSES];

/* Table of singly linked lists of free blocks for each size class */
static uint8_t *freeblocks[NUM_SIZE_CLASSES];


#ifdef MEMORY_MANAGER_STATS
/* Table of singly-linked lists of pools for each size class. Pools linked in
 * this table have no block available for allocating. */
static struct mempool *fullpools[NUM_SIZE_CLASSES];
#endif /* MEMORY_MANAGER_STATS */


// Add a mempool_t to a mempool_list_t structure as the new tail item
static void _add_pool_list_tail(mempool_t *pool, mempool_list_t *list)
{
    if (NULL == list->tail)
    {
        // First pool of this size class in the usedpools table
        list->head = pool;
        pool->prevpool = NULL;
    }
    else
    {
        list->tail->nextpool = pool;
        pool->prevpool = list->tail;
    }

    list->tail = pool;
    pool->nextpool = NULL;
}


// Initialize a freshly carved-off pool, and return a pointer to the first block
static uint8_t * _init_pool(mempool_t *pool, mempool_list_t *list, size_t size)
{
    // Add this pool to the usedpools table, using provided pointer
    _add_pool_list_tail(pool, list);

    pool->block_size = size;

    // Increment nextoffset member
    pool->nextoffset = POOL_OVERHEAD_BYTES + size;

    // Return pointer to first block in new pool
    return ((uint8_t *) pool) + POOL_OVERHEAD_BYTES;
}


// Unlink the given pool from the given list
static void _unlink_pool(mempool_t *pool, mempool_list_t *list)
{
    if (NULL != pool->nextpool)
    {
        pool->nextpool->prevpool = pool->prevpool;
    }

    if (NULL != pool->prevpool)
    {
        pool->prevpool->nextpool = pool->nextpool;
    }

    if (pool == list->head)
    {
        list->head = pool->nextpool;
    }

    if (pool == list->tail)
    {
        list->tail = pool->prevpool;
    }

#ifdef MEMORY_MANAGER_STATS
    // Add pool to fullpools list
    mempool_t **fullpool_head = &fullpools[BSTOI(pool->block_size)];
    pool->nextpool = *fullpool_head;
    *fullpool_head = pool;
#else
    pool->nextpool = NULL;
#endif /* MEMORY_MANAGER_STATS */
    pool->prevpool = NULL;
}


/* Free a block from the given pool */
static void _free_block(mempool_t *pool, void *data)
{
    uint8_t **head = &freeblocks[BSTOI(pool->block_size)];

    // Freed block now contains pointer to next free block
    *((uint8_t **) data) = *head;

    // Freed block is now the head of the free list for this pool
    *head = data;

    /* If this pool is not in the usedpools table, link it back in,
     * since it now has a free block */
    if (!POOL_IN_USE(pool))
    {
#ifdef MEMORY_MANAGER_STATS
        // Remove pool from fullpools list
        fullpools[BSTOI(pool->block_size)] = pool->nextpool;
#endif /* MEMORY_MANAGER_STATS */

        _add_pool_list_tail(pool, &usedpools[BSTOI(pool->block_size)]);
    }
}


// Pops the given block off the free list and arranges for it to be re-used
static void _reuse_freed_block(memheap_t *heap, uint8_t **freehead)
{
    mempool_t *pool = GET_POOL_POINTER(heap, *freehead);
    uint8_t *nexthead = (*(uint8_t **) *freehead);
    *freehead = nexthead;

    /* If this pool has no remaining free blocks or uncarved blocks, unlink it from
     * the usedpools table */
    if ((NULL == *freehead) && (POOL_BYTES_REMAINING(pool) < pool->block_size))
    {
        _unlink_pool(pool, &usedpools[BSTOI(pool->block_size)]);
    }
}


/* Find an available block of the given size in the given memheap_t object.
 * If a block is found, a pointer to it will be returned, otherwise NULL */
static uint8_t * _find_block(memheap_t *heap, size_t size)
{
    uint8_t *ret = NULL;
    unsigned index = BSTOI(size);


    /* Best-case scenario; there is a freed block in this size class
     * available for re-use. Let's check for that first */
    uint8_t **freehead = &freeblocks[index];

    if (NULL != *freehead)
    {
        _reuse_freed_block(heap, freehead);
        return *freehead;
    }

    /* No freed block available for re-use; next best option is to carve out a
     * new block from a pool in the usedpools table. Can we do that? */
    mempool_list_t *list = &usedpools[index];

    if (NULL == list->head)
    {
        /* Nothing in usedpools table for this size class yet; try to carve
         * out a new pool and return a pointer to the first block */
        if (HEAP_BYTES_REMAINING(heap) < POOL_SIZE_BYTES)
        {
            return NULL;
        }

        mempool_t *newpool = (mempool_t *) (heap->heap + heap->nextoffset);
        heap->nextoffset += POOL_SIZE_BYTES;
        return _init_pool(newpool, list, size);
    }

    /* Carve out a new block from the head of the usedpools list
     * for this size class */
    mempool_t *pool = list->head;
    ret = ((uint8_t *) pool) + pool->nextoffset;
    pool->nextoffset += pool->block_size;

    if (POOL_BYTES_REMAINING(pool) < pool->block_size)
    {
        // No more space in this pool-- unlink from usedpools table
        _unlink_pool(pool, list);
    }

    return ret;
}


/* Find an available block and return a pointer to it. If there are no existing
 * available blocks in the requested size class, this function will try
 * increasing the size 3 times before allocating a new memheap_t object */
static uint8_t *_small_alloc(size_t size)
{
    size_t curr_size = size;
    uint8_t *ret = NULL;
    size = ROUND_UP(size, ALIGNMENT_BYTES);

    /* Max. number of times to increment the block size class before giving up
     * and allocating a new memheap_t */
    const uint16_t max_bsz_increase = 3;

    for (memheap_t *heap = headheap; NULL != heap; heap = heap->nextheap)
    {
        for (uint16_t i = 0;
             curr_size <= MAX_BLOCK_OFFSET && i <= max_bsz_increase; i++)
        {
            ret = _find_block(heap, curr_size);
            if (NULL != ret)
            {
                return ret;
            }

            /* No used pools with enough space in this class,
             * increment and try the next class */
            curr_size += ALIGNMENT_BYTES;
        }
    }

    // Couldn't find an available block in existing heaps, allocate new tailheap
    memheap_t *newheap;

    if ((newheap = malloc(sizeof(memheap_t))) == NULL)
    {
        return NULL;
    }

    // Link new memheap_t to end of heap list
    if (NULL != tailheap)
    {
        tailheap->nextheap = newheap;
    }

    newheap->prevheap = tailheap;
    newheap->nextheap = NULL;
    tailheap = newheap;

    // Carve out new pool and return pointer to first block
    newheap->nextoffset = POOL_SIZE_BYTES;
    mempool_t *newpool = (mempool_t *) newheap->heap;
    return _init_pool(newpool, &usedpools[BSTOI(size)], size);
}


/**
 * @see memory_manager_api.h
 */
memory_manager_status_e memory_manager_init(void)
{
    if (NULL != headheap)
    {
        return MEMORY_MANAGER_ALREADY_INIT;
    }

    if ((headheap = malloc(sizeof(memheap_t))) == NULL)
    {
        return MEMORY_MANAGER_OUT_OF_MEMORY;
    }

    tailheap = headheap;

    // Zero out used pool list table
    memset(usedpools, 0, sizeof(usedpools));

    // Zero out free block list table
    memset(freeblocks, 0, sizeof(freeblocks));

#ifdef MEMORY_MANAGER_STATS
    // Zero out pointer to full pool list table
    memset(fullpools, 0, sizeof(fullpools));
#endif /* MEMORY_MANAGER_STATS */

    // Set up initial values for newly alocated heap object
    headheap->nextheap = NULL;
    headheap->prevheap = NULL;
    headheap->nextoffset = 0;

    return MEMORY_MANAGER_OK;
}

/**
 * @see memory_manager_api.h
 */
void *memory_manager_alloc(size_t size)
{
    if (SMALL_ALLOC_THRESHOLD_BYTES < size)
    {
        // size requested is large enough to use system malloc
        return malloc(size);
    }

    // size requested is small enough to use our own small heap
    return _small_alloc(size);
}


/**
 * @see memory_manager_api.h
 */
void *memory_manager_realloc(void *data, size_t size)
{
    if (NULL == data)
    {
        return memory_manager_alloc(size);
    }

    void * ret = NULL;

    // Check if source ptr is from one of our heaps
    for (memheap_t *heap = headheap; NULL != heap; heap = heap->nextheap)
    {
        if (POINTER_IN_HEAP(heap, data))
        {
            // Align down to get mempool_t pointer for this block
            mempool_t *pool = GET_POOL_POINTER(heap, data);

            // Allocate block for new size
            ret = memory_manager_alloc(size);
            if (NULL != ret)
            {
                // Copy data to new block
                memcpy(ret, data, MIN(size, pool->block_size));
            }

            // Free old data
            memory_manager_free(data);

            return ret;
        }
    }

    // Source pointer was allocated with system malloc()
    if (SMALL_ALLOC_THRESHOLD_BYTES < size)
    {
        ret = realloc(data, size);
    }
    else
    {
        // Allocate block for new size
        ret = memory_manager_alloc(size);

        if (NULL != ret)
        {
            // Copy data to new block
            memcpy(ret, data, size);
        }

        // Free old data
        memory_manager_free(data);
    }

    return ret;
}


/**
 * @see memory_manager_api.h
 */
void memory_manager_free(void *data)
{
    for (memheap_t *heap = headheap; NULL != heap; heap = heap->nextheap)
    {
        if (POINTER_IN_HEAP(heap, data))
        {
            // Align down to get mempool_t pointer for this block
            mempool_t *pool = GET_POOL_POINTER(heap, data);

            _free_block(pool, data);
            return;
        }
    }

    // Pointer is not from any of our memheap_t objects-- free with system free()
    free(data);
}


#ifdef MEMORY_MANAGER_STATS
#include <stdio.h>

/**
 * @see memory_manager_api.h
 */
memory_manager_status_e memory_manager_stats(mem_stats_t *stats)
{
    if (NULL == headheap)
    {
        return MEMORY_MANAGER_NOT_INIT;
    }

    if (NULL == stats)
    {
        return MEMORY_MANAGER_INVALID_PARAM;
    }

    unsigned total_pool_count = 0u;


    // Get heap_count
    unsigned heap_count = 0u;

    for (memheap_t *heap = headheap; NULL != heap; heap = heap->nextheap)
    {
        heap_count++;
    }

    stats->heap_count = heap_count;

    // Get full_pool_count values
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned full_count = 0u;
        for (mempool_t *curr = fullpools[i]; NULL != curr; curr = curr->nextpool)
        {
            full_count++;
            total_pool_count++;
        }

        stats->full_pool_count[i] = full_count;
    }

    // Get used_pool_count values
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned used_count = 0u;
        for (mempool_t *curr = usedpools[i].head; NULL != curr; curr = curr->nextpool)
        {
            used_count++;
            total_pool_count++;
        }

        stats->used_pool_count[i] = used_count;
    }

    stats->total_pool_count = total_pool_count;

    return MEMORY_MANAGER_OK;
}


/**
 * @see memory_manager_api.h
 */
memory_manager_status_e memory_manager_print_stats(mem_stats_t *stats)
{
    if (NULL == stats)
    {
        return MEMORY_MANAGER_INVALID_PARAM;
    }

    printf("Total heap count: %d\n", stats->heap_count);
    printf("Total pool count: %d\n\n", stats->total_pool_count);

    printf("---- Used pools ----\n\n");
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned count = stats->used_pool_count[i];
        if (0u < count)
        {
            printf("%d %d-byte pool%s\n",
                   count, ITOBS(i), (1 < count) ? "s" : "");
        }
    }

    printf("\n\n---- Full pools ----\n\n");
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned count = stats->full_pool_count[i];
        if (0u < count)
        {
            printf("%d %d-byte pool%s\n",
                   count, ITOBS(i), (1 < count) ? "s" : "");
        }
    }

    return MEMORY_MANAGER_OK;
}
#endif /* MEMORY_MANAGER_STATS */


/**
 * @see memory_manager_api.h
 */
memory_manager_status_e memory_manager_destroy(void)
{
#ifdef MEMORY_MANAGER_STATS
    mem_stats_t stats;
    memory_manager_status_e err;

    if ((err = memory_manager_stats(&stats)) != MEMORY_MANAGER_OK)
    {
        return err;
    }

    if ((err = memory_manager_print_stats(&stats)) != MEMORY_MANAGER_OK)
    {
        return err;
    }
#endif /* MEMORY_MANAGER_STATS */
    memheap_t *heap = headheap;

    while (NULL != heap)
    {
        struct memheap *nextheap = heap->nextheap;
        free(heap);
        heap = nextheap;
    }

    headheap = NULL;
    tailheap = NULL;

    return MEMORY_MANAGER_OK;
}
