#ifndef MEMORY_MANAGER_API_H
#define MEMORY_MANAGER_API_H


/* -- Beginning of tunable settings section -- */

/* If more than this number of bytes is requested, we'll just call plain
 * old system malloc(). This value needs to be a power of 2. */
#define SMALL_ALLOC_THRESHOLD_BYTES (512)


/* There are 65 size classes: 0 to 512 inclusive, in increments of 8. e.g.
 * 0, 8, 16, 24, ... and so on. This value needs to be a power of 2. */
#define ALIGNMENT_BYTES (8)


// Size of heaps, which are segmented into pools (must be a power of 2)
#define HEAP_SIZE_BYTES (256 * 1024)

// Size of pools, which are segmented into blocks (must be a power of 2)
#define POOL_SIZE_BYTES (4 * 1024)


// Map index to block size (multiply by 8 with bit shifts)
#define ITOBS(i) ((i + 1) << 3)


// Map block size to index (divide by 8 with bit shifts)
#define BSTOI(s) ((s >> 3) - 1)

/* -- End of tunable settings section -- */


// Total number of size classes used by the allocator
#define NUM_SIZE_CLASSES ((SMALL_ALLOC_THRESHOLD_BYTES / ALIGNMENT_BYTES) + 1)


// Enumeration of all status codes returned by memory_manager functions
typedef enum
{
    MEMORY_MANAGER_OK,             // Operation completed successfully
    MEMORY_MANAGER_ALREADY_INIT,   // memory_manager_init called more than once
    MEMORY_MANAGER_NOT_INIT,       // memory_manager not initialized
    MEMORY_MANAGER_INVALID_PARAM,  // Invalid parameter passed to function
    MEMORY_MANAGER_OUT_OF_MEMORY,  // System memory request failed
    MEMORY_MANAGER_ERROR           // Unspecified error
} memory_manager_status_e;


/**
 * Initialize memory manager, allocate some initial space from the system
 * memory allocator
 *
 *
 * @return    MEMORY_MANAGER_OK if memory manager was successfully initialized
 */
memory_manager_status_e memory_manager_init(void);


/**
 * Shutdown memory manager. Releases all allocated memory back to the system.
 *
 * @return    MEMORY_MANAGER_OK if memory manager was successfully shut down
 */
memory_manager_status_e memory_manager_destroy(void);


/**
 * Allocate a block of memory. If the request is larger than
 * #SMALL_ALLOC_THRESHOLD_BYTES, then it will be passed directly to system malloc()
 *
 * memory_manager_init must be called first before this function can be used
 * with predictable results
 *
 * @param  size    Size of block to request, in bytes
 *
 * @return    Pointer to allocated block, NULL if no memory could be allocated
 */
void *memory_manager_alloc(size_t size);


/**
 * Change the size of an already-allocated block of memory. Copies data if
 * necessary.
 *
 * memory_manager_init must be called first before this function can be used
 * with predictable results
 *
 * @param  data  Pointer to allocated block
 * @param  size  New size of allocated block
 *
 * @return   Pointer to new block
 */
void *memory_manager_realloc(void *data, size_t size);


/**
 * Free a block allocated by memory_manager_alloc
 *
 * memory_manager_init must be called first before this function can be used
 * with predictable results
 *
 * @param  data   Pointer to allocated block
 */
void memory_manager_free(void *data);


#ifdef MEMORY_MANAGER_STATS

// Structure to hold information returned by memory_manager_stats()
typedef struct
{
    // Total number of heap objects allocated
    unsigned total_heap_count;

    // Total number of heaps with no remaining pools to be carved off
    unsigned full_heap_count;

    // Total number of pools that have been carved off, whether in use or full
    unsigned total_pool_count;

    // Per-size-class used pool count
    unsigned used_pool_count[NUM_SIZE_CLASSES];

    // Per-size-class full pool count
    unsigned full_pool_count[NUM_SIZE_CLASSES];

    // Per-size-class free block count
    unsigned free_block_count[NUM_SIZE_CLASSES];
} mem_stats_t;


/**
 * Collect some information about the current state of memory managed by
 * memory_manager. Useful for debugging.
 *
 * @param  stats  Pointer to mem_stats_t structure to populate
 *
 * @return  MEMORY_MANAGER_OK if stats were collected successfully
 */
memory_manager_status_e memory_manager_stats(mem_stats_t *stats);

/**
 * Prints a human-readable report (to stdout) of a populated mem_stats_t
 * object
 *
 * @param  stats  Pointer to populated mem_stats_t structure to print
 *
 * @return  MEMORY_MANAGER_OK if report was printed successfully
 */
memory_manager_status_e memory_manager_print_stats(mem_stats_t *stats);

#endif /* MEMORY_MANAGER_STATS */

#endif /* MEMORY_MANAGER_API_H */
