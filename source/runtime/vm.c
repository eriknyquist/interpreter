#include <stdint.h>
#include <stdio.h>

#include "vm_api.h"
#include "opcode_handlers.h"
#include "common.h"


#define CALLSTACK_ITEMS_PER_NODE (32)
#define DATASTACK_ITEMS_PER_NODE (32)


#define CHECK_ULIST_ERR(func) {                                               \
    ulist_status_e _err_code = func;                                          \
    if (ULIST_ERROR_MEM == _err_code)                                         \
    {                                                                         \
        return VM_MEMORY_ERROR;                                               \
    }                                                                         \
    else if (ULIST_OK != _err_code)                                           \
    {                                                                         \
        return VM_ERROR;                                                      \
    }                                                                         \
}

typedef struct{
    op_handler_t handler;    // handler for opcode
    size_t bytes;            // bytecode size in bytes, excluding the opcode
} op_handler_info_t;


/* Handlers for virtual machine instructions. Arranged so that the opcode_e
 * value can be used to index the array for speedy dispatch. */
static op_handler_info_t _op_handlers[NUM_OPCODES] = {
    {.handler=opcode_handler_nop,           .bytes=0u},                 // OPCODE_NOP
    {.handler=opcode_handler_add,           .bytes=0u},                 // OPCODE_ADD
    {.handler=opcode_handler_sub,           .bytes=0u},                 // OPCODE_SUB
    {.handler=opcode_handler_mult,          .bytes=0u},                 // OPCODE_MULT
    {.handler=opcode_handler_div,           .bytes=0u},                 // OPCODE_DIV
    {.handler=opcode_handler_int,           .bytes=sizeof(vm_int_t)},   // OPCODE_INT
    {.handler=opcode_handler_float,         .bytes=sizeof(vm_float_t)}, // OPCODE_FLOAT
    {.handler=opcode_handler_string,        .bytes=0u},                 // OPCODE_STRING
    {.handler=opcode_handler_bool,          .bytes=sizeof(vm_bool_t)},  // OPCODE_BOOL
    {.handler=opcode_handler_print,         .bytes=0u},                 // OPCODE_PRINT
    {.handler=opcode_handler_cast,          .bytes=sizeof(uint16_t)},   // OPCODE_CAST
    {.handler=opcode_handler_jump,          .bytes=sizeof(int32_t)},    // OPCODE_JUMP
    {.handler=opcode_handler_jump_if_false, .bytes=sizeof(int32_t)},    // OPCODE_JUMP_IF_FALSE
    {.handler=opcode_handler_end,           .bytes=0u},                 // OPCODE_END
};


/**
 * Creates a new stack frame and intializes the data stack in the newly-created
 * frame with the correct size for data_stack_entry_t
 */
static vm_status_e init_next_callstack_frame(callstack_t *callstack)
{
    callstack_frame_t *top_frame;

    // Get pointer to the new top stack frame
    CHECK_ULIST_ERR(ulist_alloc(&callstack->frames,
                    callstack->frames.num_items, (void **) &top_frame));

    // Initialize data stack for the top callstack frame
    CHECK_ULIST_ERR(ulist_create(&top_frame->data, sizeof(data_stack_entry_t),
                                 DATASTACK_ITEMS_PER_NODE));

    callstack->current_frame = top_frame;
    return VM_OK;
}


/**
 * @see vm_api.h
 */
vm_status_e vm_create(vm_instance_t *instance)
{
    if (NULL == instance)
    {
        return VM_INVALID_PARAM;
    }

    // Create callstack
    CHECK_ULIST_ERR(ulist_create(&instance->callstack.frames,
                                 sizeof(callstack_frame_t),
                                 CALLSTACK_ITEMS_PER_NODE));

    return init_next_callstack_frame(&instance->callstack);
}


/**
 * @see vm_api.h
 */
vm_status_e vm_destroy(vm_instance_t *instance)
{
    callstack_frame_t *frame = NULL;

    CHECK_ULIST_ERR(ulist_set_iteration_start_index(&instance->callstack.frames, 0u));

    /* Iterate over callstack frames, and destroy the list used for the data
     * stack in each callstack frame */
    while (1)
    {
        ulist_status_e err;

        err = ulist_get_next_item(&instance->callstack.frames, (void **) frame);
        if (ULIST_END == err)
        {
            // No more frames in callstack
            break;
        }
        else if (ULIST_OK != err)
        {
            return VM_ERROR;
        }

        // Destroy datastack for this frame
        CHECK_ULIST_ERR(ulist_destroy(&frame->data));
    }

    // Finally, destroy the list that holds the callstack
    CHECK_ULIST_ERR(ulist_destroy(&instance->callstack.frames));

    return VM_OK;
}


vm_status_e vm_verify(opcode_t *bytecode, size_t max_bytes)
{
    uint8_t *bytes = (uint8_t *) bytecode;
    uint32_t i;

    for (i = 0; i < max_bytes; i++)
    {
        if (NUM_OPCODES <= bytes[i])
        {
            return VM_INVALID_OPCODE;
        }

        opcode_e op = (opcode_e) bytes[i];
        size_t extra_bytes_to_increment;

        if (OPCODE_STRING == op)
        {
            // Special case for string, variable bytecode length
            uint32_t string_bytes = *((uint32_t *) (bytes + i + 1u));
            extra_bytes_to_increment = sizeof(uint32_t) + string_bytes;
        }
        else
        {
            op_handler_info_t *handler_info = _op_handlers + op;
            extra_bytes_to_increment = handler_info->bytes;
        }

        i += extra_bytes_to_increment;
    }

    opcode_e last_op = bytes[i - 1];
    if (OPCODE_END != last_op)
    {
        return VM_INVALID_OPCODE;
    }

    return VM_OK;
}


/**
 * @see vm_api.h
 */
vm_status_e vm_execute(vm_instance_t *instance, opcode_t *bytecode)
{
    while (OPCODE_END != (opcode_e) *bytecode)
    {
        opcode_e op = (opcode_e) *bytecode;
        op_handler_info_t *handler_info = _op_handlers + op;

        bytecode = handler_info->handler(bytecode, instance->callstack.current_frame);
        if (NULL == bytecode)
        {
            instance->runtime_error = runtime_error_get();
            return VM_RUNTIME_ERROR;
        }
    }

    return VM_OK;
}
