#include <stdlib.h>

#include "bytecode_api.h"
#include "common.h"


// Initial size (in bytes) allocated for bytecode
#define INITIAL_SIZE (128)


// Helper macro to double the size allocated for bytecode if we run out of space
#define REQUIRE_SPACE(prog, free_bytes_needed)                                \
    do {                                                                      \
        if ((prog->total_bytes - prog->used_bytes) < free_bytes_needed)       \
        {                                                                     \
            prog->total_bytes *= 2;                                           \
            prog->bytecode = realloc(prog->bytecode, prog->total_bytes);      \
            if (NULL == prog->bytecode)                                       \
            {                                                                 \
                return BYTECODE_MEMORY_ERROR;                                 \
            }                                                                 \
        }                                                                     \
    } while(0);


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
    *((vm_int_t *) (ip + 1)) = value;

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
    *((vm_float_t *) (ip + 1)) = value;

    program->used_bytes += op_bytes;
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


bytecode_status_e bytecode_emit_end(bytecode_t *program)
{
    return _single_byte_op(program, OPCODE_END);
}
