#include <stdint.h>
#include <stdio.h>

#include "vm_api.h"
#include "common.h"
#include "type_operations_api.h"
#include "print_object_api.h"
#include "bytecode_api.h"


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


/* Handler function for opcodes */
typedef opcode_t *(*op_handler_t)(opcode_t *, callstack_frame_t *);



static opcode_t *_arithmetic_op(opcode_t *opcode, callstack_frame_t *frame,
                                arith_type_e arith_type)
{
    data_stack_entry_t lhs, rhs, result;

    CHECK_ULIST_ERR(ulist_pop_item(&frame->data, frame->data.num_items - 1, &rhs), NULL);
    CHECK_ULIST_ERR(ulist_pop_item(&frame->data, frame->data.num_items - 1, &lhs), NULL);

    type_status_e err = type_arithmetic(&lhs.payload.object, &rhs.payload.object,
                                        &result.payload.object, arith_type);

    if (TYPE_OK != err)
    {
        RUNTIME_ERR("Can't do that arithmetic\n");
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


static opcode_t *_handle_int(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    entry.payload.data_object.object.obj_type = OBJTYPE_DATA;
    entry.payload.data_object.data_type = DATATYPE_INT;

    // Increment past the opcode
    opcode += 1;

    // Grab int value after opcode
    entry.payload.data_object.payload.int_value = *((vm_int_t *) opcode);

    // Push int value onto stack
    CHECK_ULIST_ERR(ulist_append_item(&frame->data, &entry), NULL);

    // Increment past the int value
    return ADVANCE_IP(opcode, sizeof(vm_int_t));
}


static opcode_t *_handle_float(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    entry.payload.data_object.object.obj_type = OBJTYPE_DATA;
    entry.payload.data_object.data_type = DATATYPE_FLOAT;

    // Increment past the opcode
    opcode += 1;

    // Grab int value after opcode
    entry.payload.data_object.payload.float_value = *((vm_float_t *) opcode);

    // Push int value onto stack
    CHECK_ULIST_ERR(ulist_append_item(&frame->data, &entry), NULL);

    // Increment past the int value
    return ADVANCE_IP(opcode, sizeof(vm_float_t));
}


static opcode_t *_handle_print(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    CHECK_ULIST_ERR(ulist_pop_item(&frame->data, frame->data.num_items - 1, &entry), NULL);

    print_object(&entry.payload.object);

    return opcode + 1;
}


static opcode_t *_handle_end(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode;
}


static op_handler_t _op_handlers[NUM_OPCODES] = {
    _handle_nop,              // OPCODE_NOP
    _handle_add,              // OPCODE_ADD
    _handle_sub,              // OPCODE_SUB
    _handle_mult,             // OPCODE_MULT
    _handle_div,              // OPCODE_DIV
    _handle_int,              // OPCODE_INT
    _handle_float,            // OPCODE_FLOAT
    _handle_print,            // OPCODE_PRINT
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
            RUNTIME_ERR("Unrecognised opcode %d\n", *bytecode);
            return VM_ERROR;
        }

        opcode_e op = (opcode_e) *bytecode;
        op_handler_t handler = _op_handlers[op];

        bytecode = handler(bytecode, instance->callstack.current_frame);
        if (NULL == bytecode)
        {
            RUNTIME_ERR("Error executing instruction %d\n", op);
            return VM_ERROR;
        }
    }

    return VM_OK;
}