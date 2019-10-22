#ifndef BYTECODE_API_H_
#define BYTECODE_API_H_

#include <stdio.h>
#include <stdint.h>

#include "data_types.h"


typedef enum
{
    BYTECODE_OK,
    BYTECODE_INVALID_PARAM,
    BYTECODE_MEMORY_ERROR,
    BYTECODE_ERROR
} bytecode_status_e;


/* Type of an encoded opcode */
typedef uint8_t opcode_t;


typedef struct
{
    opcode_t *bytecode;
    size_t total_bytes;
    size_t used_bytes;
} bytecode_t;


/* Enumeration of all valid opcode values */
typedef enum
{
    OPCODE_NOP,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_MULT,
    OPCODE_DIV,
    OPCODE_INT,
    OPCODE_FLOAT,
    OPCODE_PRINT,
    OPCODE_END,
    NUM_OPCODES
} opcode_e;


bytecode_status_e bytecode_create(bytecode_t *program);


bytecode_status_e bytecode_destroy(bytecode_t *program);


void bytecode_dump_raw(bytecode_t *program);


bytecode_status_e bytecode_emit_int(bytecode_t *program, vm_int_t value);


bytecode_status_e bytecode_emit_float(bytecode_t *program, vm_float_t value);


bytecode_status_e bytecode_emit_add(bytecode_t *program);


bytecode_status_e bytecode_emit_sub(bytecode_t *program);


bytecode_status_e bytecode_emit_mult(bytecode_t *program);


bytecode_status_e bytecode_emit_div(bytecode_t *program);


bytecode_status_e bytecode_emit_print(bytecode_t *program);


bytecode_status_e bytecode_emit_end(bytecode_t *program);


#endif /* BYTECODE_API_H_ */
