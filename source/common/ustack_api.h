#ifndef USTACK_API_H
#define USTACK_API_H

/**
 * Enumeration of status codes returned by ustack API functions
 */
typedef enum
{
    USTACK_OK,
    USTACK_EMPTY,
    USTACK_INVALID_PARAM,
    USTACK_MEMORY_ERROR,
    USTACK_ERROR,
} ustack_status_e;

typedef struct ustack_node ustack_node_t;

/**
 * Structure representing a single node in an unrolled stack
 */
struct ustack_node
{
    struct ustack_node *next;    // Pointer to next node
    struct ustack_node *prev;    // Pointer to previous node
    size_t start;                // Index of first populated item in node
    size_t count;                // Number of items contained in node
    char data[];                 // Pointer to first item
};


/**
 * Structure representing an unrolled stack instance
 */
typedef struct
{
    size_t count;
    size_t items_per_node;
    size_t item_size_bytes;
    ustack_node_t *head;
    ustack_node_t *tail;
} ustack_t;


/**
 * Intialize an unrolled stack instance. No memory will be allocated for stack
 * data until data items are actually added.
 *
 * @param stack            Pointer to ustack_t instance
 * @param items_per_node   Number of items per stack node
 * @param item_size_bytes  Item size in bytes
 *
 * @param USTACK_OK if stack was initialized successfully
 */
ustack_status_e ustack_create(ustack_t *stack, size_t items_per_node,
                              size_t item_size_bytes);


/**
 * Add a new item at the tail of the given stack.
 *
 * @param stack   Pointer to ustack_t instance
 * @param item    Pointer to item data to add
 *
 * @param USTACK_OK if item was added successfully
 */
ustack_status_e ustack_put(ustack_t *stack, void *item);


/**
 * Retrieves and removes the first item (the item at the head) of the given
 * stack. Retrieving items using this function provides FIFO-like behaviour
 * (first-in-first-out).
 *
 * @param stack   Pointer to ustack_t instance
 * @param item    Pointer to location to store head item data
 *
 * @param USTACK_OK if item was added successfully
 */
ustack_status_e ustack_get_first(ustack_t *stack, void *item);


/**
 * Retrieves pointer to the first item (the item at the head) of the given stack.
 *
 * @param stack   Pointer to ustack_t instance
 * @param item    Pointer to location to store pointer to head item
 *
 * @param USTACK_OK if item pointer was retrieved successfully
 */
ustack_status_e ustack_peek_first(ustack_t *stack, void **item);


/**
 * Retrieves and removes the last item (the item at the tail) of the given
 * stack. Retrieving items using this function provides LIFO-like behaviour
 * (last-in-first-out).
 *
 * @param stack   Pointer to ustack_t instance
 * @param item    Pointer to location to store tail item data
 *
 * @param USTACK_OK if item was added successfully
 */
ustack_status_e ustack_get_last(ustack_t *stack, void *item);


/**
 * Retrieves pointer to the last item (the item at the tail) of the given stack.
 *
 * @param stack   Pointer to ustack_t instance
 * @param item    Pointer to location to store pointer to tail item
 *
 * @param USTACK_OK if item pointer was retrieved successfully
 */
ustack_status_e ustack_peek_last(ustack_t *stack, void **item);

#endif /* USTACK_API_H */
