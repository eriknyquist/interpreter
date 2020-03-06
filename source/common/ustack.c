#include <stdlib.h>
#include <string.h>

#include "ustack_api.h"


// Min. allowable value for items_per_node parameter
#define MIN_ITEMS_PER_NODE  (2)

// Helper macro to calculate the size of a single node in bytes for stack 's'
#define NODE_SIZE_BYTES(s) (sizeof(ustack_node_t) + ((s)->item_size_bytes * \
                                                    (s)->items_per_node))

// Get a pointer to the first item in node 'n', from stack 's'
#define FIRST_IN_NODE(s, n) (n->data + (s->item_size_bytes * n->start))

// Get a pointer to the last item in node 'n', from stack 's'
#define LAST_IN_NODE(s, n) (n->data + \
                            (s->item_size_bytes * (n->start + n->count - 1)))

// Get a pointer to the tail item of stack 's' (item count must be non-zero)
#define TAIL_ITEM_PTR(s) (LAST_IN_NODE(s, s->tail))

// Get a pointer to the head item of stack 's' (item count must be non-zero)
#define HEAD_ITEM_PTR(s) (FIRST_IN_NODE(s, s->head))

// Get number of free items in the node 'n', from stack 's'
#define NODE_FREE_ITEMS(s, n) (s->items_per_node - (n->start + n->count))


/**
 * Allocate a new node and link it into the given stack as the new tail node
 */
static ustack_status_e _new_tail_node(ustack_t *stack)
{
    ustack_node_t *newnode;

    if ((newnode = malloc(NODE_SIZE_BYTES(stack))) == NULL)
    {
        return USTACK_MEMORY_ERROR;
    }

    newnode->start = 0u;
    newnode->count = 0u;
    newnode->next = NULL;
    newnode->prev = NULL;

    if (NULL == stack->head)
    {
        stack->head = newnode;
        stack->tail = newnode;
    }
    else
    {
        newnode->prev = stack->tail;
        stack->tail->next = newnode;
        stack->tail = newnode;
    }

    return USTACK_OK;
}


/**
 * @see ustack_api.h
 */
ustack_status_e ustack_create(ustack_t *stack, size_t items_per_node,
                              size_t item_size_bytes)
{
    if ((NULL == stack) || (MIN_ITEMS_PER_NODE > items_per_node))
    {
        return USTACK_INVALID_PARAM;
    }

    stack->items_per_node = items_per_node;
    stack->item_size_bytes = item_size_bytes;
    stack->count = 0u;
    stack->head = NULL;
    stack->tail = NULL;

    return USTACK_OK;
}


/**
 * @see ustack_api.h
 */
ustack_status_e ustack_put(ustack_t *stack, void *item)
{
    if ((NULL == stack) || (NULL == item))
    {
        return USTACK_INVALID_PARAM;
    }

    if ((NULL == stack->head) || (0u == NODE_FREE_ITEMS(stack, stack->tail)))
    {
        ustack_status_e err;
        if (USTACK_OK != (err = _new_tail_node(stack)))
        {
            return err;
        }
    }

    memcpy(TAIL_ITEM_PTR(stack), item, stack->item_size_bytes);
    stack->tail->count++;
    stack->count++;

    return USTACK_OK;
}


/**
 * @see ustack_api.h
 */
ustack_status_e ustack_get_first(ustack_t *stack, void *item)
{
    if ((NULL == stack) || (NULL == item))
    {
        return USTACK_INVALID_PARAM;
    }

    if (NULL == stack->head)
    {
        return USTACK_EMPTY;
    }

    memcpy(item, HEAD_ITEM_PTR(stack), stack->item_size_bytes);
    stack->head->start++;

    // If the head node is now empty, free it
    if (stack->items_per_node == NODE_FREE_ITEMS(stack, stack->head))
    {
        ustack_node_t *node_to_free = stack->head;

        if (NULL == stack->head->next)
        {
            // Last node
            stack->head = NULL;
            stack->tail = NULL;
        }
        else
        {
            // Not the last node
            stack->head->next->prev = NULL;
            stack->head = stack->head->next;
        }

        free(node_to_free);
    }

    stack->count--;
    return USTACK_OK;
}


/**
 * @see ustack_api.h
 */
ustack_status_e ustack_peek_first(ustack_t *stack, void **item)
{
    if ((NULL == stack) || (NULL == item))
    {
        return USTACK_INVALID_PARAM;
    }

    if (NULL == stack->head)
    {
        return USTACK_EMPTY;
    }

    *item = HEAD_ITEM_PTR(stack);
    return USTACK_OK;
}


/**
 * @see ustack_api.h
 */
ustack_status_e ustack_get_last(ustack_t *stack, void *item)
{
    if ((NULL == stack) || (NULL == item))
    {
        return USTACK_INVALID_PARAM;
    }

    if (NULL == stack->head)
    {
        return USTACK_EMPTY;
    }

    memcpy(item, TAIL_ITEM_PTR(stack), stack->item_size_bytes);
    stack->head->count--;

    // If the tail node is now empty, free it
    if (stack->items_per_node == NODE_FREE_ITEMS(stack, stack->tail))
    {
        ustack_node_t *node_to_free = stack->tail;

        if (NULL == stack->tail->prev)
        {
            // Last node
            stack->head = NULL;
            stack->tail = NULL;
        }
        else
        {
            // Not the last node
            stack->tail->prev->next = NULL;
            stack->tail = stack->tail->prev;
        }

        free(node_to_free);
    }

    stack->count--;
    return USTACK_OK;
}


/**
 * @see ustack_api.h
 */
ustack_status_e ustack_peek_last(ustack_t *stack, void **item)
{
    if ((NULL == stack) || (NULL == item))
    {
        return USTACK_INVALID_PARAM;
    }

    if (NULL == stack->head)
    {
        return USTACK_EMPTY;
    }

    *item = TAIL_ITEM_PTR(stack);
    return USTACK_OK;
}
