/*
 * Fixed-size block allocator for small allocations ( < 512 bytes). Heavily
 * influenced by Python's obmalloc.c, although far simpler.
 *
 * We allocate memory from the system in chunks of HEAP_SIZE_BYTES, and we refer
 * to this largest unit of memory as a "heap" ("arena" seems to be the common
 * term for similar chunks in cpython and malloc implementations, but whatever).
 *
 * Heaps are carved into fixed-size pools of POOL_SIZE_BYTES. All pools are the
 * same size, but can contain different block sizes. Valid block sizes range from
 * 8 to 512 bytes, in multiples of 8 (e.g. 8, 16, 24, 32, ... 504, 512). This
 * means there are 64 size classes in total. The housekeeping data for a pool is
 * stored within the area allocated for the pool itself, such that the space
 * reserved for blocks within a pool is actually:
 *
 * POOL_SIZE_BYTES - ROUND_UP(sizeof(mempool_t), ALIGNMENT_BYTES)
 *
 * This is not ideal, as it results in most pools having an unused partial block
 * (an area for possible future improvement).
 *
 * Pools are then further carved into blocks of a fixed size. No housekeeping
 * data is required for blocks that are in use, although when a block is freed
 * the block space is re-used to hold a pointer to the next free block (more
 * about this later).
 *
 *
 * Consider the following operations:
 *
 *   - memory_manager_init()
 *
 *   - block1 = memory_manager_alloc(8);
 *
 *   - block2 = memory_manager_alloc(8);
 *
 *   - memory_manager_free(block1);
 *
 *
 * The diagram below illustrates what the allocated heap structure looks like
 * after these operations (note that the pool size is 4096 bytes, and
 * ROUND_UP(sizeof(mempool_t), ALIGNMENT_BYTES) is 24 bytes in the diagram
 * below):
 *
 *   heap start, byte 0
 *   -------------------->  +-------------------------------+
 *                          |                               |
 *                          |          memheap_t            |
 *   pool 0 start, byte 24  |                               |
 *   -------------------->  +-------------------------------+
 *                          |          block1 (freed)       |
 *                          +-------------------------------+
 *                          |          block2 (in use)      |
 *                          +-------------------------------+
 *                          |          unused block         |
 *                          +-------------------------------+
 *                          |                               |
 *                          |             .....             |
 *                          |                               |
 *   start of unused pool   +-------------------------------+
 *   space, byte 4120       |          unused block         |
 *   -------------------->  +-------------------------------+
 *                          |                               |
 *                          |             .....             |
 *   heap end, byte 262168  |                               |
 *   -------------------->  +-------------------------------+
 *
 *
 *  Heap management
 *  ---------------
 *
 *  Heaps are allocated as needed and maintained in a doubly-linked list
 *  ("usedheaps" list). When the last pool is carved off a heap, it is removed
 *  from the usedheaps list. When there are no remaining heaps in the usedheaps
 *  list, a new heap is allocated and added to the end of this list (more about
 *  this in 'Allocation strategy').
 *
 *
 *  Pool management
 *  ---------------
 *
 *  For each size class, we maintain a doubly-linked list of all pools in that
 *  size class (across all heaps) with remaining unused blocks-- that is, virgin
 *  blocks that have never been touched ("usedpools" table).
 *  When the last unused block in a pool is carved out, it will be removed
 *  from the usedpools table. The only way we can use blocks in this pool after
 *  this point is through the freeblocks table, if any blocks in the pool are
 *  freed (more about this in the next section).
 *
 *
 *  Block management
 *  ----------------
 *
 *  For each size class, we maintain a singly-linked list of all free blocks in
 *  that size class, in all heaps ("freeblocks" table). When a block is freed,
 *  it becomes the head of the free list for its size class, which is achieved
 *  by re-using the initial bytes of the freed block to contain a pointer to the
 *  next free block.
 *
 *
 *  Allocation strategy
 *  -------------------
 *
 *  Like python's obmalloc.c, we do our best to avoid touching a piece of memory
 *  until it is needed. Pools and blocks are only carved out as-needed to satisfy
 *  requests.
 *
 *  Requests larger than SMALL_ALLOC_THRESHOLD_BYTES are just passed
 *  directly to system malloc(). For smaller requests, we will first look in
 *  freeblocks for a freed block in the same size class that we can re-use. If
 *  this fails, we'll see if usedpools contains any pools in the same size class
 *  that we can carve a new block out of. If this also fails, we will try to
 *  carve out a new pool (of the originally requested block size) out of the head
 *  heap in usedheaps (if the head of usedheaps is NULL, we will allocate a
 *  new heap).
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
#define POOL_IN_USE(pool) ((NULL != pool->next) || (NULL != pool->prev))


/* Helper macro to unlink an item from doubly-linked list structures
 * mempool_list_t and memheap_list_t */
#define LIST_UNLINK(item, list)                     \
{                                                   \
    if (NULL != (item)->next)                       \
    {                                               \
        (item)->next->prev = (item)->prev;          \
    }                                               \
                                                    \
    if (NULL != (item)->prev)                       \
    {                                               \
        (item)->prev->next = (item)->next;          \
    }                                               \
                                                    \
    if ((item) == (list)->head)                     \
    {                                               \
        (list)->head = (item)->next;                \
    }                                               \
                                                    \
    if ((item) == (list)->tail)                     \
    {                                               \
        (list)->tail = (item)->prev;                \
    }                                               \
                                                    \
    (item)->next = NULL;                            \
    (item)->prev = NULL;                            \
}


/* Helper macro to add a new tail item to doubly-linked list structures
 * mempool_list_t and memheap_list_t */
#define LIST_LINK_TAIL(item, list)                  \
{                                                   \
    if (NULL == (list)->tail)                       \
    {                                               \
        (list)->head = item;                        \
        (item)->prev = NULL;                        \
    }                                               \
    else                                            \
    {                                               \
        (list)->tail->next = item;                  \
        (item)->prev = (list)->tail;                \
    }                                               \
                                                    \
    (list)->tail = item;                            \
    (item)->next = NULL;                            \
}                                                   \


#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define MAX(x, y) (((x) > (y)) ? (x) : (y))


/* Structure representing a pool containing blocks of a single size class.
 *
 * This structure is overlaid at the beginning of a pool, such that the space
 * available for object allocation in a pool is actually
 * POOL_SIZE_BYTES - ROUND_UP(sizeof(mempool_t), ALIGNMENT_BYTES) */
typedef struct mempool
{
    struct mempool *next; // pointer to next pool of this size class
    struct mempool *prev; // pointer to previous pool of this size class

    /* offset in bytes of next unused block in this pool, from the
     * beginning of this struct */
    uint16_t nextoffset;

    // size of blocks stored in this pool
    uint16_t block_size;
} mempool_t;


/* Stucture representing a heap, containing a fixed number of pools */
typedef struct memheap
{
    struct memheap *next;
    struct memheap *prev;

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


/* Structure representing a doubly-linked list of memheap_t structures */
typedef struct
{
    memheap_t *head;
    memheap_t *tail;
} memheap_list_t;


/* Doubly-linked list of memheap_t objects with pools yet to be carved off */
static memheap_list_t usedheaps;

/* Table of doubly-linked lists of pools for each size class. Pools linked in
 * this table have blocks yet to be carved off. */
static mempool_list_t usedpools[NUM_SIZE_CLASSES];

/* Table of singly linked lists of free blocks for each size class */
static uint8_t *freeblocks[NUM_SIZE_CLASSES];


#ifdef MEMORY_MANAGER_STATS
/* Table of singly-linked lists of pools for each size class. Pools linked in
 * this table have no remaining uncarved blocks. */
static struct mempool *fullpools[NUM_SIZE_CLASSES];
#endif /* MEMORY_MANAGER_STATS */

/* Singly-linked list of full heaps. Heaps linked in this list have no
 * remaining uncarved pools. */
static memheap_t *fullheaps = NULL;


/* A list of lists of heaps. All lists containing allocated memheap_t objects
 * of any state should be referenced here-- we'll used this list to 1) make sure
 * we can find the right heap for pointers passed to memory_manager_free, and 2)
 * make sure we can free all allocated memheap_t objects in memory_manager_destroy */
#define NUM_HEAPLISTS (2u)
static memheap_t **heaplists[NUM_HEAPLISTS] = {&usedheaps.head, &fullheaps};


// Initialize a freshly carved-off pool, and return a pointer to the first block
static uint8_t * _init_pool(mempool_t *pool, mempool_list_t *list, size_t size)
{
    // Add this pool to the usedpools table, using provided pointer
    LIST_LINK_TAIL(pool, list);

    pool->block_size = size;

    // Increment nextoffset member
    pool->nextoffset = POOL_OVERHEAD_BYTES + size;

    // Return pointer to first block in new pool
    return ((uint8_t *) pool) + POOL_OVERHEAD_BYTES;
}


/* Tries first to re-use a freed block for the requested size if possible, and
 * then try to carve out a new block from a used pool. If both fail, returns NULL,
 * otherwise a pointer to the available block is returned */
static uint8_t *_find_block_in_used_pool(size_t size)
{
    uint8_t *ret = NULL;
    unsigned index = BSTOI(size);

    /* Best-case scenario; there is a freed block in this size class
     * available for re-use. Let's check for that first */
    uint8_t **freehead = &freeblocks[index];
    if (NULL != *freehead)
    {
        /* Pop the head block off the free list. The block we're re-using contains
         * a pointer to next free block, and that next free block is now the head
         * of this free list */
        uint8_t *oldhead = *freehead;
        *freehead = (*(uint8_t **) *freehead);
        return oldhead;
    }

    /* No freed block available for re-use; next best option is to carve out a
     * new block from a pool in the usedpools table. Can we do that? */
    mempool_list_t *list = &usedpools[index];
    if (NULL == list->head)
    {
        return NULL;
    }

    /* Carve out a new block from the head of the usedpools list
     * for this size class */
    mempool_t *pool = list->head;
    ret = ((uint8_t *) pool) + pool->nextoffset;
    pool->nextoffset += pool->block_size;

    if (POOL_BYTES_REMAINING(pool) < pool->block_size)
    {
        // No more space in this pool-- unlink from usedpools table
        LIST_UNLINK(pool, list);
#ifdef MEMORY_MANAGER_STATS
        // Add pool to fullpools list
        mempool_t **fullpool_head = &fullpools[BSTOI(pool->block_size)];
        pool->next = *fullpool_head;
        *fullpool_head = pool;
#endif /* MEMORY_MANAGER_STATS */
    }

    return ret;
}


static uint8_t *_find_new_pool(size_t size)
{
    memheap_t *heap = usedheaps.head;
    mempool_t *newpool = (mempool_t *) (heap->heap + heap->nextoffset);
    heap->nextoffset += POOL_SIZE_BYTES;

    if (HEAP_BYTES_REMAINING(heap) < POOL_SIZE_BYTES)
    {
        // Unlink this heap from doubly-linked usedheaps list
        LIST_UNLINK(heap, &usedheaps);

        // Add heap to singly-linked fullheaps list
        heap->next = fullheaps;
        fullheaps = heap;
    }

    return _init_pool(newpool, &usedpools[BSTOI(size)], size);
}


/* Walk through all heaps in usedheaps and fullheaps to find the heap containing
 * the provided pointer, and return a pointer to the heap. If none of our heaps
 * contain the pointer, it must have been allocated using system malloc(),
 * so return NULL */
static memheap_t *_find_heap_for_pointer(void *data)
{
    for (unsigned i = 0; i < NUM_HEAPLISTS; i++)
    {
        // Check if ptr is from one of the heaps in this list
        for (memheap_t *heap = *(heaplists[i]); NULL != heap; heap = heap->next)
        {
            if (POINTER_IN_HEAP(heap, data))
            {
                return heap;
            }
        }
    }

    return NULL;
}


/**
 * @see memory_manager_api.h
 */
memory_manager_status_e memory_manager_init(void)
{
    if (NULL != usedheaps.head)
    {
        return MEMORY_MANAGER_ALREADY_INIT;
    }

    // Zero out used heaps list structure
    memset(&usedheaps, 0, sizeof(usedheaps));

    // Zero out used pool list table
    memset(usedpools, 0, sizeof(usedpools));

    // Zero out free block list table
    memset(freeblocks, 0, sizeof(freeblocks));

#ifdef MEMORY_MANAGER_STATS
    // Zero out pointer to full pool list table
    memset(fullpools, 0, sizeof(fullpools));
#endif /* MEMORY_MANAGER_STATS */

    if ((usedheaps.head = malloc(sizeof(memheap_t))) == NULL)
    {
        return MEMORY_MANAGER_OUT_OF_MEMORY;
    }

    usedheaps.tail = usedheaps.head;

    // Set up initial values for newly allocated heap object
    usedheaps.head->next = NULL;
    usedheaps.head->prev = NULL;
    usedheaps.head->nextoffset = 0;

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

    uint8_t *ret = NULL;
    size = ROUND_UP(size, ALIGNMENT_BYTES);

    // Try to find a block in in a pool that's already in use
    if ((ret = _find_block_in_used_pool(size)) != NULL)
    {
        return ret;
    }

    // Try to carve out a new pool in existing heaps
    if ((usedheaps.head) && ((ret = _find_new_pool(size)) != NULL))
    {
        return ret;
    }

    // Couldn't satisfy request with existing heaps, allocate new heap
    memheap_t *newheap;

    if ((newheap = malloc(sizeof(memheap_t))) == NULL)
    {
        return NULL;
    }

    // Link new memheap_t to end of heap list
    LIST_LINK_TAIL(newheap, &usedheaps);

    // Carve out new pool and return pointer to first block
    newheap->nextoffset = POOL_SIZE_BYTES;
    mempool_t *newpool = (mempool_t *) newheap->heap;
    return _init_pool(newpool, &usedpools[BSTOI(size)], size);
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
    memheap_t *heap;

    if ((heap = _find_heap_for_pointer(data)) != NULL)
    {
        // Align down to get mempool_t pointer for this block
        mempool_t *pool = GET_POOL_POINTER(heap, data);

        // Allocate block for new size
        ret = memory_manager_alloc(size);
        if (NULL == ret)
        {
            return NULL;
        }

        // Copy data to new block
        memcpy(ret, data, MIN(size, pool->block_size));

        // Free old data
        memory_manager_free(data);

        return ret;
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
    memheap_t *heap;

    if ((heap = _find_heap_for_pointer(data)) != NULL)
    {
        // Align down to get mempool_t pointer for this block
        mempool_t *pool = GET_POOL_POINTER(heap, data);

        unsigned index = BSTOI(pool->block_size);
        uint8_t **head = &freeblocks[index];

        // Freed block now contains pointer to next free block
        *((uint8_t **) data) = *head;

        // Freed block is now the head of the free list for this pool
        *head = data;

        return;
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
    if (NULL == usedheaps.head)
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

    for (memheap_t *heap = usedheaps.head; NULL != heap; heap = heap->next)
    {
        heap_count++;
    }

    // Get full_heap_count
    unsigned full_heap_count = 0u;

    for (memheap_t *heap = fullheaps; NULL != heap; heap = heap->next)
    {
        full_heap_count++;
    }

    stats->total_heap_count = heap_count + full_heap_count;
    stats->full_heap_count = full_heap_count;

    // Get full_pool_count values
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned full_count = 0u;
        for (mempool_t *curr = fullpools[i]; NULL != curr; curr = curr->next)
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
        for (mempool_t *curr = usedpools[i].head; NULL != curr; curr = curr->next)
        {
            used_count++;
            total_pool_count++;
        }

        stats->used_pool_count[i] = used_count;
    }

    stats->total_pool_count = total_pool_count;

    // Get free_block_count values
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned block_count = 0u;
        for (uint8_t *head = freeblocks[i]; NULL != head; head = *((uint8_t **) head))
        {
            block_count++;
        }

        stats->free_block_count[i] = block_count;
    }

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

    printf("Total heap count: %d\n", stats->total_heap_count);
    printf("Full heap count: %d\n", stats->full_heap_count);
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

    printf("\n---- Free blocks ----\n\n");
    for (unsigned i = 0; i < NUM_SIZE_CLASSES; i++)
    {
        unsigned count = stats->free_block_count[i];
        if (0u < count)
        {
            printf("%d %d-byte block%s\n",
                   count, ITOBS(i), (1 < count) ? "s" : "");
        }
    }

    printf("\n");
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

    for (unsigned i = 0; i < NUM_HEAPLISTS; i++)
    {
        memheap_t *heap = *(heaplists[i]);
        while (NULL != heap)
        {
            struct memheap *next = heap->next;
            free(heap);
            heap = next;
        }
    }

    usedheaps.head = NULL;
    usedheaps.tail = NULL;
    fullheaps = NULL;

    return MEMORY_MANAGER_OK;
}
