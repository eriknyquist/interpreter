#include <stdint.h>
#include <stdio.h>

#include "data_types.h"
#include "type_operations_api.h"


#define CALLSTACK_ITEMS_PER_NODE (32)
#define DATASTACK_ITEMS_PER_NODE (32)


#define CHECK_ULIST_ERR(func, return_err) { \
    ulist_status_e _err_code = func;        \
    if (ULIST_ERROR_MEM == _err_code)       \
    {                                       \
        return return_err;                  \
    }                                       \
    else if (ULIST_OK != _err_code)         \
    {                                       \
        return return_err;                  \
    }                                       \
}


#define RUNTIME_ERR(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)


/**
 * Structure representing all object types that can be pushed to the data stack
 */
typedef struct
{
    union
    {
        object_t object;
        data_object_t data_object;
    } payload;
} data_stack_entry_t;


/* Enumeration of all valid opcode values */
typedef enum
{
    OPCODE_NOP,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MULT,
    OPCODE_DIV,
    OPCODE_CONST,
    OPCODE_END,
    NUM_OPCODES
} opcode_e;


/* Enumeration of status codes returned by virtual machine functions */
typedef enum
{
    VM_OK,
    VM_MEMORY_ERROR,
    VM_INVALID_PARAM,
    VM_ERROR
} vm_status_e;


/* Type of an encoded opcode */
typedef uint8_t opcode_t;


/* Handler function for opcodes */
typedef opcode_t *(*op_handler_t)(opcode_t *, callstack_frame_t *);


/* Structure for data representing a VM instance */
typedef struct
{
    callstack_t callstack;
} vm_instance_t;


static opcode_t *_arithmetic_op(opcode_t *opcode, callstack_frame_t *frame,
                                arith_type_e arith_type)
{
    data_stack_entry_t lhs, rhs, result;

    CHECK_ULIST_ERR(ulist_pop_item(&frame->data, frame->data.num_items, &rhs), NULL);
    CHECK_ULIST_ERR(ulist_pop_item(&frame->data, frame->data.num_items, &lhs), NULL);

    type_status_e err = type_arithmetic(&lhs.payload.object, &rhs.payload.object,
                                        &result.payload.object, arith_type);

    if (TYPE_OK != err)
    {
        RUNTIME_ERR("Can't do that arithmetic");
        return NULL;
    }

    CHECK_ULIST_ERR(ulist_append_item(&frame->data, &result), NULL);
    return opcode + 1;
}


static opcode_t *_handle_nop(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode + 1;
}


static opcode_t *_handle_add(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_ADD);
}


static opcode_t *_handle_sub(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_SUB);
}


static opcode_t *_handle_mult(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_MULT);
}


static opcode_t *_handle_div(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_DIV);
}


static opcode_t *_handle_const(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode + 1;
}


static opcode_t *_handle_end(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode + 1;
}


static op_handler_t _op_handlers[NUM_OPCODES] = {
    _handle_nop,              // OPCODE_NOP
    _handle_add,              // OPCODE_ADD
    _handle_sub,              // OPCODE_SUB
    _handle_mult,             // OPCODE_MULT
    _handle_div,              // OPCODE_DIV
    _handle_const,            // OPCODE_CONST
    _handle_end               // OPCODE_END
};


static vm_status_e init_next_callstack_frame(callstack_t *callstack)
{
    callstack_frame_t *top_frame;

    // Get pointer to the top stack frame
    CHECK_ULIST_ERR(ulist_alloc(&callstack->frames,
                    callstack->frames.num_items, (void **) &top_frame), VM_ERROR);

    // Initialize data stack for the top callstack frame
    CHECK_ULIST_ERR(ulist_create(&top_frame->data, sizeof(data_stack_entry_t),
                                 DATASTACK_ITEMS_PER_NODE), VM_ERROR);

    callstack->current_frame = top_frame;
    return VM_OK;
}


vm_status_e vm_create(vm_instance_t *instance)
{
    if (NULL == instance)
    {
        return VM_INVALID_PARAM;
    }

    // Create callstack
    CHECK_ULIST_ERR(ulist_create(&instance->callstack.frames,
                                 sizeof(callstack_frame_t),
                                 CALLSTACK_ITEMS_PER_NODE), VM_MEMORY_ERROR);

    return init_next_callstack_frame(&instance->callstack);
}


vm_status_e vm_execute(vm_instance_t *instance, opcode_t *bytecode)
{
    while (OPCODE_END != (opcode_e) *bytecode)
    {
        if (NUM_OPCODES <= *bytecode)
        {
            RUNTIME_ERR("Unrecognised opcode %d", *bytecode);
            return VM_ERROR;
        }

        opcode_e op = (opcode_e) *bytecode;
        op_handler_t handler = _op_handlers[op];

        bytecode = handler(bytecode, instance->callstack.current_frame);
        if (NULL == bytecode)
        {
            RUNTIME_ERR("Error executing instruction %d", op);
            return VM_ERROR;
        }
    }

    return VM_OK;
}
