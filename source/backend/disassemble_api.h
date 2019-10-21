#ifndef DISASSEMBLE_API_H_
#define DISASSEMBLE_API_H_


#include "bytecode_api.h"


typedef enum
{
    DISASSEMBLE_OK,
    DISASSEMBLE_INVALID_PARAM,
    DISASSEMBLE_BYTECODE_INVALID,
    DISASSEMBLE_ERROR
} disassemble_status_e;


disassemble_status_e disassemble_bytecode(bytecode_t *program);


#endif /* DISASSEMBLE_API_H_ */
