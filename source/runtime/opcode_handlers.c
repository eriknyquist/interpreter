#include "print_object_api.h"
#include "bytecode_api.h"
#include "ulist_api.h"
#include "opcode_handlers.h"
#include "common.h"


#define CHECK_ULIST_ERR_RT(func)                                              \
    do {                                                                      \
        ulist_status_e _err_code = func;                                      \
        if (ULIST_OK != _err_code)                                            \
        {   RUNTIME_ERR(RUNTIME_ERROR_MEMORY,                                 \
                        "ulist_t operation failed, status %d", _err_code);    \
            return NULL;                                                      \
        }                                                                     \
    }                                                                         \
    while(0);


static opcode_t *_arithmetic_op(opcode_t *opcode, callstack_frame_t *frame,
                                    arith_type_e arith_type)
{
    data_stack_entry_t lhs, rhs, result;

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &rhs));
    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &lhs));

    type_status_e err = type_arithmetic(&lhs.payload.object, &rhs.payload.object,
                                        &result.payload.object, arith_type);
    if (TYPE_RUNTIME_ERROR == err)
    {
        return NULL;
    }
    else if (TYPE_OK != err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC, "Can't do that arithmetic\n");
        return NULL;
    }

    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &result));
    return opcode + 1;
}


opcode_t *opcode_handler_nop(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode + 1;
}


opcode_t *opcode_handler_add(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_ADD);
}


opcode_t *opcode_handler_sub(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_SUB);
}


opcode_t *opcode_handler_mult(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_MULT);
}


opcode_t *opcode_handler_div(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_DIV);
}


opcode_t *opcode_handler_int(opcode_t *opcode, callstack_frame_t *frame)
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


opcode_t *opcode_handler_float(opcode_t *opcode, callstack_frame_t *frame)
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


opcode_t *opcode_handler_string(opcode_t *opcode, callstack_frame_t *frame)
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


opcode_t *opcode_handler_print(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &entry));

    print_object(&entry.payload.object);

    return INCREMENT_PTR_BYTES(opcode, 1);
}


opcode_t *opcode_handler_end(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode;
}
