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


/* Handlers for virtual machine instructions. Arranged so that the opcode_e
 * value can be used to index the array for speedy dispatch. */
static op_handler_t _op_handlers[NUM_OPCODES] = {
    opcode_handler_nop,              // OPCODE_NOP
    opcode_handler_add,              // OPCODE_ADD
    opcode_handler_sub,              // OPCODE_SUB
    opcode_handler_mult,             // OPCODE_MULT
    opcode_handler_div,              // OPCODE_DIV
    opcode_handler_int,              // OPCODE_INT
    opcode_handler_float,            // OPCODE_FLOAT
    opcode_handler_string,           // OPCODE_STRING
    opcode_handler_bool,             // OPCODE_BOOL
    opcode_handler_print,            // OPCODE_PRINT
    opcode_handler_cast,             // OPCODE_CAST
    opcode_handler_jump,             // OPCODE_JUMP
    opcode_handler_jump_if_false,    // OPCODE_JUMP_IF_FALSE
    opcode_handler_end               // OPCODE_END
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


/**
 * @see vm_api.h
 */
vm_status_e vm_execute(vm_instance_t *instance, opcode_t *bytecode)
{
    while (OPCODE_END != (opcode_e) *bytecode)
    {
        if (NUM_OPCODES <= *bytecode)
        {
            RUNTIME_ERR(RUNTIME_ERROR_INVALID_OPCODE,
                        "Unrecognised opcode %d\n", *bytecode);
            return VM_RUNTIME_ERROR;
        }

        opcode_e op = (opcode_e) *bytecode;
        op_handler_t handler = _op_handlers[op];

        bytecode = handler(bytecode, instance->callstack.current_frame);
        if (NULL == bytecode)
        {
            instance->runtime_error = runtime_error_get();
            return VM_RUNTIME_ERROR;
        }
    }

    return VM_OK;
}
