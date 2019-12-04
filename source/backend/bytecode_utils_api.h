#ifndef BYTECODE_UTILS_API_H
#define BYTECODE_UTILS_API_H


#include "data_types.h"
#include "bytecode_common.h"

#include <stddef.h>


/**
 * Given a pointer to an immediate data value encoded in bytecode,
 * return the size in bytes of the data object
 *
 * @param    opcode    Pointer to encoded immediate data value
 * @return             Size in bytes of immediate data value
 */
size_t bytecode_utils_data_object_size_bytes(opcode_t *opcode);


/**
 * Given a pointer to a populated data_object_t struct, return the size in bytes
 * of the data object if it were encoded in bytecode
 *
 * @param    opcode    Pointer to encoded immediate data value
 * @return             Size in bytes of data_object_t if it were encoded
 */
size_t bytecode_utils_data_object_encoded_size_bytes(data_object_t *data_obj);


#endif /* BYTECODE_UTILS_API_H */
