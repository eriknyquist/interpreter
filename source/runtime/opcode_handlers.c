#include <string.h>

#include "print_object_api.h"
#include "bytecode_api.h"
#include "ulist_api.h"
#include "opcode_handlers.h"
#include "common.h"
#include "string_cache_api.h"


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
opcode_t *opcode_handler_nop(opcode_t *opcode, vm_instance_t *instance)
{
    return opcode + 1;
}


/**
 * Pops two items off the stack, adds them together and creates a new value
 * containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_add(opcode_t *opcode, vm_instance_t *instance)
{
    return _arithmetic_op(opcode, instance->callstack.current_frame, ARITH_ADD);
}


/**
 * Pops two items off the stack, subtracts the first popped from the second popped
 * and creates a new value containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_sub(opcode_t *opcode, vm_instance_t *instance)
{
    return _arithmetic_op(opcode, instance->callstack.current_frame, ARITH_SUB);
}


/**
 * Pops two items off the stack, multiplies them and creates a new value
 * containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_mult(opcode_t *opcode, vm_instance_t *instance)
{
    return _arithmetic_op(opcode, instance->callstack.current_frame, ARITH_MULT);
}


/**
 * Pops two items off the stack, divides the first popped by the second popped
 * and creates a new value containing the result, and pushes the result to the stack
 *
 * 0000  opcode  (1 byte)
 */
opcode_t *opcode_handler_div(opcode_t *opcode, vm_instance_t *instance)
{
    return _arithmetic_op(opcode, instance->callstack.current_frame, ARITH_DIV);
}


/**
 * Creates an  object with the provided integer value and pushes it to the stack
 *
 * 0000  opcode    (1 byte)
 * 0001  int value (4 bytes, signed integer)
 */
opcode_t *opcode_handler_int(opcode_t *opcode, vm_instance_t *instance)
{
    data_stack_entry_t entry;
    callstack_frame_t *frame = instance->callstack.current_frame;

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
opcode_t *opcode_handler_float(opcode_t *opcode, vm_instance_t *instance)
{
    data_stack_entry_t entry;
    callstack_frame_t *frame = instance->callstack.current_frame;

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
opcode_t *opcode_handler_string(opcode_t *opcode, vm_instance_t *instance)
{
    data_stack_entry_t entry;
    callstack_frame_t *frame = instance->callstack.current_frame;

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
    CHECK_STRING_CACHE_ERR_RT(string_cache_add((char *) opcode, string_size, &string));

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
opcode_t *opcode_handler_bool(opcode_t *opcode, vm_instance_t *instance)
{
    data_stack_entry_t entry;
    callstack_frame_t *frame = instance->callstack.current_frame;

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
opcode_t *opcode_handler_print(opcode_t *opcode, vm_instance_t *instance)
{
    data_stack_entry_t entry;
    callstack_frame_t *frame = instance->callstack.current_frame;

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
 *       base of the source string from 2-36 (as specified by standard docs for strtol)
 *
 *     - when converting float to string, it is used to specify the number of digits
 *       after the decimal point that should be included in the output string.
 */
opcode_t *opcode_handler_cast(opcode_t *opcode, vm_instance_t *instance)
{
    callstack_frame_t *frame = instance->callstack.current_frame;
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
 * Jump unconditionally to the given offset in the program data
 *
 * 0000  opcode                                   (1 byte)
 * 0001  offset (in bytes) from current position  (4 bytes, signed integer)
 */
opcode_t *opcode_handler_jump(opcode_t *opcode, vm_instance_t *instance)
{
    int32_t offset = *((int32_t *) (((uint8_t *) opcode) + 1));
    return INCREMENT_PTR_BYTES(opcode, offset);
}


/**
 * Pop a value from the stack, and cast it to a bool. If false, jump to the
 *   given offset in the program data.
 *
 * 0000  opcode                                   (1 byte)
 * 0001  offset (in bytes) from current position  (4 bytes, signed integer)
 */
opcode_t *opcode_handler_jump_if_false(opcode_t *opcode, vm_instance_t *instance)
{
    callstack_frame_t *frame = instance->callstack.current_frame;
    data_object_t bool_data, *bool_data_ptr;
    data_stack_entry_t entry;
    type_status_e err;

    CHECK_ULIST_ERR_RT(ulist_pop_item(&frame->data, frame->data.num_items - 1, &entry));

    // Cast popped item to bool
    err = type_cast_to(&entry.payload.object, &bool_data.object,
                       DATATYPE_BOOL, 0u);

    if (TYPE_NO_CAST_REQUIRED == err)
    {
        bool_data_ptr = &entry.payload.data_object;
    }
    else if (TYPE_OK == err)
    {
        bool_data_ptr = &bool_data;
    }
    else
    {
        if (TYPE_RUNTIME_ERROR != err)
        {
            RUNTIME_ERR(RUNTIME_ERROR_CAST, "Failed to cast, status %d\n", err);
        }

        return NULL;
    }

    opcode_t *ret;

    if (bool_data_ptr->payload.bool_value)
    {
        // Increment past the opcode and offset data
        ret = INCREMENT_PTR_BYTES(opcode, 1 + sizeof(int32_t));
    }
    else
    {
        // Jump to the given offset
        int32_t offset = *((int32_t *) (((uint8_t *) opcode) + 1));
        ret = INCREMENT_PTR_BYTES(opcode, offset);
    }

    return ret;
}


/**
 * Appends a new value to the constant pool
 *
 * 0000  opcode          (1 byte)
 * 0001  data type       (1 byte, unsigned integer, values defined in in data_type_e)
 * 0002  immediate value (size depends on the value of data type field)
 */
opcode_t *opcode_handler_define_const(opcode_t *opcode, vm_instance_t *instance)
{
    data_object_t new_const;

    // Increment past the opcode
    opcode = INCREMENT_PTR_BYTES(opcode, 1);

    new_const.object.obj_type = OBJTYPE_DATA;
    new_const.data_type = (data_type_e) *((uint8_t *) opcode);

    // Increment past data type
    opcode = INCREMENT_PTR_BYTES(opcode, 1);

    switch (new_const.data_type)
    {
        case DATATYPE_INT:
            new_const.payload.int_value = *((vm_int_t *) opcode);
            opcode = INCREMENT_PTR_BYTES(opcode, sizeof(vm_int_t));
            break;

        case DATATYPE_FLOAT:
            new_const.payload.float_value = *((vm_float_t *) opcode);
            opcode = INCREMENT_PTR_BYTES(opcode, sizeof(vm_float_t));
            break;

        case DATATYPE_BOOL:
            new_const.payload.bool_value = *((vm_bool_t *) opcode);
            opcode = INCREMENT_PTR_BYTES(opcode, sizeof(vm_bool_t));
            break;

        case DATATYPE_STRING:
        {
            // Read size of the upcoming string data
            uint32_t string_size = *((uint32_t *) opcode);

            // Increment past the string size
            opcode = INCREMENT_PTR_BYTES(opcode, sizeof(uint32_t));

            // Create new byte string object
            byte_string_t *new_string;
            CHECK_STRING_CACHE_ERR_RT(string_cache_add((char *) opcode, string_size, &new_string));

            // Copy new byte_string_t object to new const object
            memcpy(&new_const.payload.string_value, new_string, sizeof(byte_string_t));

            // Increment past string data
            opcode = INCREMENT_PTR_BYTES(opcode, string_size);
            break;
        }

        default:
            break;
    }

    // Append new const value to the constants pool
    CHECK_ULIST_ERR_RT(ulist_append_item(&instance->constants, &new_const));

    return opcode;
}


/**
 * Loads a value from the constants pool and pushes it to the stack
 *
 * 0000  opcode   (1 byte)
 * 0001  index    (4 bytes, unsigned integer, const pool index to load from)
 */
opcode_t *opcode_handler_load_const(opcode_t *opcode, vm_instance_t *instance)
{
    callstack_frame_t *frame = instance->callstack.current_frame;
    data_stack_entry_t entry;

    // Increment past the opcode
    opcode = INCREMENT_PTR_BYTES(opcode, 1);

    uint32_t index = *((uint32_t *) opcode);
    entry.payload.object.obj_type = OBJTYPE_DATA;

    CHECK_ULIST_ERR_RT(ulist_get_item(&instance->constants,
                                      (unsigned long long) index,
                                      (void **) &entry.payload.data_object));

    // Push loaded constant onto stack
    CHECK_ULIST_ERR_RT(ulist_append_item(&frame->data, &entry));

    // Increment past the index
    return INCREMENT_PTR_BYTES(opcode, sizeof(uint32_t));
}


/**
 * Currently, does nothing except act as sentintel to let the VM know that there
 * are no more instructions to execute
 *
 * 0000  opcode      (1 byte)
 */
opcode_t *opcode_handler_end(opcode_t *opcode, vm_instance_t *instance)
{
    return opcode;
}
