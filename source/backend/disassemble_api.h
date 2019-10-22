#ifndef DISASSEMBLE_API_H_
#define DISASSEMBLE_API_H_


#include "bytecode_api.h"


/* Enumeration of all values returned by disassemble_api functions */
typedef enum
{
    DISASSEMBLE_OK,
    DISASSEMBLE_INVALID_PARAM,
    DISASSEMBLE_BYTECODE_INVALID,
    DISASSEMBLE_ERROR
} disassemble_status_e;


/**
 * Disassemble bytecode and print it in a human-readable format
 *
 * @param   program           Pointer to the bytecode_t stucture to disassemble
 * @param   num_instructions  Number of instructions to disassemble (0 for all)
 *
 * @return  DISASSEMBLE_OK if bytecode was disassembled successfully
 */
disassemble_status_e disassemble_bytecode(bytecode_t *program, size_t num_instructions);


#endif /* DISASSEMBLE_API_H_ */
