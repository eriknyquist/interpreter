/**
 * Functions for generating bytecode
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "bytecode_api.h"
#include "bytecode_utils_api.h"
#include "common.h"


// Initial size (in bytes) allocated for bytecode
#define INITIAL_SIZE (128)


// Helper macro to double the size allocated for bytecode if we run out of space
#define REQUIRE_SPACE(prog, free_bytes_needed)                                \
    do {                                                                      \
        if ((prog->total_bytes - prog->used_bytes) < (free_bytes_needed))     \
        {                                                                     \
            prog->total_bytes *= 2;                                           \
            prog->bytecode = realloc(prog->bytecode, prog->total_bytes);      \
            if (NULL == prog->bytecode)                                       \
            {                                                                 \
                return BYTECODE_MEMORY_ERROR;                                 \
            }                                                                 \
        }                                                                     \
    } while(0)


bytecode_status_e bytecode_create(bytecode_t *program)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    if ((program->bytecode = malloc(INITIAL_SIZE)) == NULL)
    {
        return BYTECODE_MEMORY_ERROR;
    }

    program->total_bytes = INITIAL_SIZE;
    program->used_bytes = 0;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_destroy(bytecode_t *program)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    program->total_bytes = 0;
    program->used_bytes = 0;
    free(program->bytecode);

    return BYTECODE_OK;
}


void bytecode_dump_raw(bytecode_t *program)
{
    for (size_t i = 0; i < program->used_bytes; i++)
    {
        printf("%02x ", program->bytecode[i]);
    }

    printf("\n");
}


bytecode_status_e bytecode_emit_int(bytecode_t *program, vm_int_t value)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = sizeof(vm_int_t) + 1;
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;
    *ip = (opcode_t) OPCODE_INT;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *((vm_int_t *) ip) = value;

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_float(bytecode_t *program, vm_float_t value)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = sizeof(vm_float_t) + 1;
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;
    *ip = (opcode_t) OPCODE_FLOAT;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *((vm_float_t *) ip) = value;

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_string(bytecode_t *program, char *value)
{
    if ((NULL == program) || (NULL == value))
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t string_bytes = strlen(value);
    if (string_bytes > UINT32_MAX)
    {
        return BYTECODE_ERROR;
    }

    size_t op_bytes = sizeof(uint32_t) + string_bytes + 1;
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;
    *ip = (opcode_t) OPCODE_STRING;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *((uint32_t *) ip) = (uint32_t) string_bytes;

    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, sizeof(uint32_t));
    (void) memcpy(ip, value, string_bytes);

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_bool(bytecode_t *program, vm_bool_t value)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = sizeof(vm_bool_t) + 1;
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;
    *ip = (opcode_t) OPCODE_BOOL;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *((vm_bool_t *) ip) = value;

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


/* 'data' is only used in two cases:
 *
 * - if we are casting from float to string, 'data' is the number of
 *   places after the decimal point to add in the created string object.
 * - if we are casting from int to string, 'data' is the numerical base that
 *   the string should be interpreted in (expected value is between 2-36, this
 *   is passed directly to strtol so a better description of different bases can
 *   be found here: http://man7.org/linux/man-pages/man3/strtol.3.html\
 */
bytecode_status_e bytecode_emit_cast(bytecode_t *program, data_type_e data_type,
                                     uint16_t data)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = 1 + sizeof(uint8_t) + sizeof(uint16_t);
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;

    *ip = (opcode_t) OPCODE_CAST;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *ip = (uint8_t) data_type;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, sizeof(uint8_t));

    *((uint16_t *) ip) = data;

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_jump(bytecode_t *program, int32_t offset)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = 1 + sizeof(int32_t);
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;

    *ip = (opcode_t) OPCODE_JUMP;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *((int32_t *) ip) = offset;

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_jump_if_false(bytecode_t *program, int32_t offset)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = 1 + sizeof(int32_t);
    REQUIRE_SPACE(program, op_bytes);

    opcode_t *ip = program->bytecode + program->used_bytes;

    *ip = (opcode_t) OPCODE_JUMP_IF_FALSE;
    ip = (opcode_t *) INCREMENT_PTR_BYTES(ip, 1);

    *((int32_t *) ip) = offset;

    program->used_bytes += op_bytes;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_backpatched_jump(bytecode_t *program,
                                                 uint32_t *position)
{
    if (NULL == position)
    {
        return BYTECODE_INVALID_PARAM;
    }

    *position = program->used_bytes;
    (void) bytecode_emit_jump(program, 0);
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_backpatched_jump_if_false(bytecode_t *program,
                                                          uint32_t *position)
{
    if (NULL == position)
    {
        return BYTECODE_INVALID_PARAM;
    }

    *position = program->used_bytes;
    (void) bytecode_emit_jump_if_false(program, 0);
    return BYTECODE_OK;
}


bytecode_status_e bytecode_backpatch_jump(bytecode_t *program, uint32_t position,
                                          int32_t offset)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    uint8_t *patch_location = ((uint8_t *) program->bytecode) + position;

    // Sanity check on patch location
    opcode_e opcode = (opcode_e) *patch_location;
    if ((OPCODE_JUMP != opcode) && (OPCODE_JUMP_IF_FALSE != opcode))
    {
        return BYTECODE_INVALID_BACKPATCH;
    }

    // +1 byte to skip the opcode
    patch_location += 1u;

    // Insert the new offset value
    *((int32_t *) patch_location) = offset;

    return BYTECODE_OK;
}


static bytecode_status_e _single_byte_op(bytecode_t *program, opcode_e op)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    REQUIRE_SPACE(program, 1);

    program->bytecode[program->used_bytes] = (opcode_t) op;
    program->used_bytes += 1;
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_add(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_ADD);
}


bytecode_status_e bytecode_emit_sub(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_SUB);
}


bytecode_status_e bytecode_emit_mult(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_MULT);
}


bytecode_status_e bytecode_emit_div(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_DIV);
}


bytecode_status_e bytecode_emit_print(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_PRINT);
}


bytecode_status_e bytecode_emit_define_const(bytecode_t *program,
                                             data_type_e datatype, void *data)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }


    // 2 bytes for opcode and data type
    REQUIRE_SPACE(program, 2);
    program->bytecode[program->used_bytes] = OPCODE_DEFINE_CONST;
    program->bytecode[program->used_bytes + 1] = datatype;
    program->used_bytes += 2;

    size_t data_bytes;

    switch (datatype)
    {
        case DATATYPE_INT:
            data_bytes = sizeof(vm_int_t);
            break;

        case DATATYPE_FLOAT:
            data_bytes = sizeof(vm_float_t);
            break;

        case DATATYPE_BOOL:
            data_bytes = sizeof(vm_bool_t);
            break;

        case DATATYPE_STRING:
            REQUIRE_SPACE(program, sizeof(uint32_t));
            data_bytes = strlen(data);
            *((uint32_t *) (program->bytecode + program->used_bytes)) = (uint32_t) data_bytes;
            program->used_bytes += sizeof(uint32_t);
            break;
        default:
            return BYTECODE_ERROR;
    }

    memcpy(program->bytecode + program->used_bytes, data, data_bytes);
    program->used_bytes += data_bytes;

    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_load_const(bytecode_t *program, uint32_t index)
{
    if (NULL == program)
    {
        return BYTECODE_INVALID_PARAM;
    }

    size_t op_bytes = 1 + sizeof(uint32_t);
    REQUIRE_SPACE(program, op_bytes);

    program->bytecode[program->used_bytes] = (opcode_t) OPCODE_LOAD_CONST;
    program->used_bytes += 1;

    *((uint32_t *) (program->bytecode + program->used_bytes)) = index;
    program->used_bytes += sizeof(uint32_t);
    return BYTECODE_OK;
}


bytecode_status_e bytecode_emit_end(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_END);
}
