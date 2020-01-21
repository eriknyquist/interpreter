#include "bytecode_utils_api.h"
#include "data_types.h"
#include "common.h"


/**
 * @see bytecode_utils_api.h
 */
size_t bytecode_utils_data_object_size_bytes(opcode_t *opcode)
{
    size_t ret;

    data_type_e datatype = (data_type_e) *((uint8_t *) opcode);

    // Increment past the data type
    opcode = INCREMENT_PTR_BYTES(opcode, 1);

    switch (datatype)
    {
        case DATATYPE_INT:
            ret = 1u + sizeof(vm_int_t);
            break;

        case DATATYPE_FLOAT:
            ret = 1u + sizeof(vm_float_t);
            break;

        case DATATYPE_BOOL:
            ret = 1u + sizeof(vm_bool_t);
            break;

        case DATATYPE_STRING:
        {
            uint32_t string_bytes = *((uint32_t *) opcode);
            ret = 1u + sizeof(uint32_t) + string_bytes;
        }
            break;

        default:
            ret = 1u;
            break;
    }

    return ret;
}


/**
 * @see bytecode_utils_api.h
 */
size_t bytecode_utils_data_object_encoded_size_bytes(data_object_t *data_obj)
{
    size_t ret;

    switch (data_obj->data_type)
    {
        case DATATYPE_INT:
            ret = 1u + sizeof(vm_int_t);
            break;

        case DATATYPE_FLOAT:
            ret = 1u + sizeof(vm_float_t);
            break;

        case DATATYPE_BOOL:
            ret = 1u + sizeof(vm_bool_t);
            break;

        case DATATYPE_STRING:
        {
            ret = 1u + sizeof(uint32_t) + data_obj->payload.string_value.size;
        }
            break;

        default:
            ret = 1u;
            break;
    }

    return ret;
}

