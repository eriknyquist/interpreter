#include <stdio.h>

#include "disassemble_api.h"
#include "data_types.h"


disassemble_status_e disassemble_bytecode(bytecode_t *program)
{
    if (NULL == program)
    {
        return DISASSEMBLE_INVALID_PARAM;
    }

    uint8_t *ip;
    size_t bytes_consumed = 0;

    while (bytes_consumed < program->used_bytes)
    {
        ip = program->bytecode + bytes_consumed;
        printf("%08lx ", bytes_consumed);

        switch ((opcode_e)  *ip)
        {
            case OPCODE_NOP:
                printf("NOP");
                bytes_consumed += 1;
                break;

            case OPCODE_ADD:
                printf("ADD");
                bytes_consumed += 1;
                break;

            case OPCODE_SUB:
                printf("SUB");
                bytes_consumed += 1;
                break;

            case OPCODE_MULT:
                printf("MULT");
                bytes_consumed += 1;
                break;

            case OPCODE_DIV:
                printf("ADD");
                bytes_consumed += 1;
                break;

            case OPCODE_INT:
                printf("INT %d", *((vm_int_t *) (ip + 1)));
                bytes_consumed += sizeof(vm_int_t) + 1;
                break;

            case OPCODE_FLOAT:
                printf("FLOAT %.4f", *((vm_float_t *) (ip + 1)));
                bytes_consumed += sizeof(vm_float_t) + 1;
                break;

            case OPCODE_PRINT:
                printf("PRINT");
                bytes_consumed += 1;
                break;

            case OPCODE_END:
                printf("END");
                bytes_consumed += 1;
                break;

            default:
                printf("????");
                bytes_consumed += 1;
                break;
        }

        printf("\n");
    }

    return DISASSEMBLE_OK;
}
