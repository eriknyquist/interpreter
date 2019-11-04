#include "print_object_api.h"
#include "bytecode_api.h"
#include "ulist_api.h"
#include "opcode_handlers.h"
#include "common.h"
#include "runtime_common.h"


/* Boilerplate for performing an arithmetic operation by popping two operands
 * off the stack and pushing the result to the stack */
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

    FREE_DATA_OBJECT(&lhs.payload.object);
    FREE_DATA_OBJECT(&rhs.payload.object);

    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &result));
    return opcode + 1;
}


/**
 * Does nothing for a single cycle of the virtual machine
 *
 * opcode format:
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_nop(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode + 1;
}


/**
 * Pops two items off the stack, adds them together and creates a new value
 * containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_add(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_ADD);
}


/**
 * Pops two items off the stack, subtracts the first popped from the second popped
 * and creates a new value containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_sub(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_SUB);
}


/**
 * Pops two items off the stack, multiplies them and creates a new value
 * containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_mult(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_MULT);
}


/**
 * Pops two items off the stack, divides the first popped by the second popped
 * and creates a new value containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_div(opcode_t *opcode, callstack_frame_t *frame)
{
    return _arithmetic_op(opcode, frame, ARITH_DIV);
}


/**
 * Creates an  object with the provided integer value and pushes it to the stack
 *
 * 0000  opcode    (1 byte)
 * 0001  int value (4 bytes, signed integer)
 */
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

    // Increment past the int value and return
    return INCREMENT_PTR_BYTES(opcode, sizeof(vm_int_t));
}


/**
 * Creates a float object with the provided floating point value and pushes it
 * to the stack
 *
 * 0000  opcode      (1 byte)
 * 0001  float value (8 bytes, double-precision float)
 */
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

    // Increment past the float value and return
    return INCREMENT_PTR_BYTES(opcode, sizeof(vm_float_t));
}


/**
 * Creates a string object with the provided string data and pushes it to the stack
 *
 * 0000  opcode                       (1 byte)
 * 0001  size of string data in bytes (4 bytes, unsigned integer)
 * 0005  string data                  (no. of bytes specified by previous field)
 */
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

    CHECK_BYTESTR_ERR_RT(byte_string_create(string));

    /* Read string data into byte string object (+1 to ensure we copy the
     * trailing null byte as well) */
    CHECK_BYTESTR_ERR_RT(byte_string_add_bytes(string, string_size + 1, opcode));

    // Push byte string value onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &entry));

    // Increment past string data
    return INCREMENT_PTR_BYTES(opcode, string_size);
}




/**
 * Creates a bool object with the provided boolean value and pushes it to the stack
 *
 * 0000  opcode      (1 byte)
 * 0001  bool value  (1 byte, unsigned integer, 1=true 0=false)
 */
opcode_t *opcode_handler_bool(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    entry.payload.data_object.object.obj_type = OBJTYPE_DATA;
    entry.payload.data_object.data_type = DATATYPE_BOOL;

    // Increment past the opcode
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, 1);

    // Grab bool value after opcode
    entry.payload.data_object.payload.bool_value = *((vm_bool_t *) opcode);

    // Push new bool value onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &entry));

    // Increment past the bool value and return
    return INCREMENT_PTR_BYTES(opcode, sizeof(vm_bool_t));
}


/**
 * Pop an item off the stack and prints it to stdout
 *
 * 0000  opcode      (1 byte)
 */
opcode_t *opcode_handler_print(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t entry;

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &entry));

    print_object(&entry.payload.object);

    return INCREMENT_PTR_BYTES(opcode, 1);
}


/**
 * Attempts to create a new item by popping an item of the stack and casting it
 * to the specified data type
 *
 * 0000  opcode      (1 byte)
 * 0001  data type   (1 byte, unsigned integer, values defined in in data_type_e)
 * 0002  extra data  (2 bytes, unsigned integer, details below)
 *
 * details for extra data:
 *
 *     extra data is only used in two situations,
 *
 *     - when converting string to int, it is used to specify the numerical
 *       base of the source string from 2-36 (as specified by standard snprintf docs)
 *
 *     - when converting float to string, it is used to specify the number of digits
 *       after the decimal point that should be included in the output string.
 */
opcode_t *opcode_handler_cast(opcode_t *opcode, callstack_frame_t *frame)
{
    data_stack_entry_t input;
    data_stack_entry_t output;
    type_status_e err;

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &input));

    // Increment past the opcode
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, 1);

    // Read data type
    uint8_t data_type_u8 = *((uint8_t *) opcode);
    data_type_e data_type = (data_type_e) data_type_u8;

    // Increment past the data type
    opcode = (opcode_t *) INCREMENT_PTR_BYTES(opcode, 1);

    // Read extra data
    uint16_t extra_data = *((uint16_t *) opcode);

    err = type_cast_to(&input.payload.object, &output.payload.object,
                       data_type, extra_data);

    if (TYPE_OK != err)
    {
        if (TYPE_RUNTIME_ERROR != err)
        {
            RUNTIME_ERR(RUNTIME_ERROR_CAST, "Failed to cast, status %d", err);
        }

        return NULL;
    }

    // Push result of cast onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &output));

    // Increment past extra data and return
    return INCREMENT_PTR_BYTES(opcode, sizeof(uint16_t));
}


/**
 * Currently, does nothing except act as sentintel to let the VM know that there
 * are no more instructions to execute
 *
 * 0000  opcode      (1 byte)
 */
opcode_t *opcode_handler_end(opcode_t *opcode, callstack_frame_t *frame)
{
    return opcode;
}
