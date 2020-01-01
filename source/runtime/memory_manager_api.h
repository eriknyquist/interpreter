#ifndef MEMORY_MANAGER_API_H
#define MEMORY_MANAGER_API_H


/* If more than this number of bytes is requested, we'll just call plain
 * old system malloc(). This value needs to be a power of 2. */
#define SMALL_ALLOC_THRESHOLD_BYTES (512)


/* There are 65 size classes: 0 to 512 inclusive, in increments of 8. e.g.
 * 0, 8, 16, 24, ... and so on. */
#define ALIGNMENT_BYTES (8)


// Size of heaps, which are segmented into pools
#define HEAP_SIZE_BYTES (256 * 1024)

// Size of pools, which are segmented into blocks
#define POOL_SIZE_BYTES (4 * 1024)


// Map index to block size (multiply by 8 with bit shifts)
#define ITOBS(i) ((i + 1) << 3)


// Map block size to index
#define BSTOI(s) ((s >> 3) - 1)


// Total number of size classes used by the allocator
#define NUM_SIZE_CLASSES ((SMALL_ALLOC_THRESHOLD_BYTES / ALIGNMENT_BYTES) + 1)


typedef enum
{
    MEMORY_MANAGER_OK,
    MEMORY_MANAGER_ALREADY_INIT,
    MEMORY_MANAGER_INVALID_PARAM,
    MEMORY_MANAGER_OUT_OF_MEMORY,
    MEMORY_MANAGER_ERROR
} memory_manager_status_e;


/**
 * Initialize memory manager, allocate some initial space from the system
 * memory allocator
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
 * @param  size    Size of block to request, in bytes
 *
 * @return    Pointer to allocated block, NULL if no memory could be allocated
 */
void *memory_manager_alloc(size_t size);


/**
 * Change the size of an already-allocated block of memory. Copies data if
 * necessary.
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
 * @param  data   Pointer to allocated block
 */
void memory_manager_free(void *data);


#endif
