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


#endif /* BYTECODE_UTILS_API_H */
