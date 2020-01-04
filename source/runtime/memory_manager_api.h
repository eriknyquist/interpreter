#ifndef MEMORY_MANAGER_API_H
#define MEMORY_MANAGER_API_H


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
