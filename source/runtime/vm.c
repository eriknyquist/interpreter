#include <stdint.h>
#include <stdio.h>

#include "vm_api.h"
#include "common.h"
#include "type_operations_api.h"
#include "print_object_api.h"
#include "bytecode_api.h"


#define CALLSTACK_ITEMS_PER_NODE (32)
#define DATASTACK_ITEMS_PER_NODE (32)


#define RUNTIME_ERR(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)


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


#define CHECK_ULIST_ERR_RT(func)                                              \
    do {                                                                      \
        ulist_status_e _err_code = func;                                      \
        if (ULIST_OK != _err_code)                                            \
        {   RUNTIME_ERR("ulist_t operation failed, status %d", _err_code);    \
            return NULL;                                                      \
        }                                                                     \
    }                                                                         \
    while(0);


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

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &rhs));
    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &lhs));

    type_status_e err = type_arithmetic(&lhs.payload.object, &rhs.payload.object,
                                        &result.payload.object, arith_type);

    if (TYPE_OK != err)
    {
        RUNTIME_ERR("Can't do that arithmetic\n");
        return NULL;
    }

    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &result));
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
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, 1);

    // Grab int value after opcode
    entry.payload.data_object.payload.int_value = *((vm_int_t *) opcode);

    // Push int value onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &entry));

    // Increment past the int value
    return INCREMENT_PTR_BYTES(opcode, sizeof(vm_int_t));
}


static opcode_t *_handle_float(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    entry.payload.data_object.object.obj_type = OBJTYPE_DATA;
    entry.payload.data_object.data_type = DATATYPE_FLOAT;

    // Increment past the opcode
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, 1);

    // Grab int value after opcode
    entry.payload.data_object.payload.float_value = *((vm_float_t *) opcode);

    // Push int value onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &entry));

    // Increment past the int value
    return INCREMENT_PTR_BYTES(opcode, sizeof(vm_float_t));
}


static opcode_t *_handle_string(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    entry.payload.data_object.object.obj_type = OBJTYPE_DATA;
    entry.payload.data_object.data_type = DATATYPE_STRING;

    // Increment past the opcode
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, 1);

    // Read size of the upcoming string data
    uint32_t string_size = *(uint32_t *) opcode;

    // Increment past the string size
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, sizeof(uint32_t));

    // Set up new byte string object
    byte_string_t *string = &entry.payload.data_object.payload.string_value;

    if (BYTE_STRING_OK != byte_string_create(string))
    {
        return NULL;
    }

    /* Read string data into byte string object (+1 to ensure we copy the
     * trailing null byte as well) */
    if (BYTE_STRING_OK != byte_string_add_bytes(string, opcode, string_size + 1))
    {
        return NULL;
    }

    // Push byte string value onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &entry));

    // Increment past string data
    return INCREMENT_PTR_BYTES(opcode, string_size);
}


static opcode_t *_handle_print(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &entry));

    print_object(&entry.payload.object);

    return INCREMENT_PTR_BYTES(opcode, 1);
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
    _handle_string,           // OPCODE_STRING
    _handle_print,            // OPCODE_PRINT
    _handle_end               // OPCODE_END
};


static vm_status_e init_next_callstack_frame(callstack_t *callstack)
{
    callstack_frame_t *top_frame;

    // Get pointer to the top stack frame
    CHECK_ULIST_ERR(ulist_alloc(&callstack->frames,
                    callstack->frames.num_items, (void **) &top_frame));

    // Initialize data stack for the top callstack frame
    CHECK_ULIST_ERR(ulist_create(&top_frame->data, sizeof(data_stack_entry_t),
                                 DATASTACK_ITEMS_PER_NODE));

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
                                 CALLSTACK_ITEMS_PER_NODE));

    return init_next_callstack_frame(&instance->callstack);
}


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
