#include <stdio.h>

#include "byte_string_api.h"
#include "disassemble_api.h"
#include "data_types.h"


const char * _datatype_name(data_type_e data_type)
{
    switch (data_type)
    {
        case DATATYPE_INT:
            return "INT";
            break;

        case DATATYPE_FLOAT:
            return "FLOAT";
            break;

        case DATATYPE_STRING:
            return "STRING";
            break;

        default:
            return "????";
            break;
    }

    return "????";
}


disassemble_status_e disassemble_bytecode(bytecode_t *program, size_t num_instructions)
{
    if (NULL == program)
    {
        return DISASSEMBLE_INVALID_PARAM;
    }

    uint8_t *ip;
    int32_t offset;
    size_t bytes_consumed = 0;
    size_t instructions_consumed = 0;

    while (bytes_consumed < program->used_bytes)
    {
        if (num_instructions && (instructions_consumed >= num_instructions))
        {
            break;
        }

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
                printf("DIV");
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

            case OPCODE_STRING:
                ip += 1;

                uint32_t string_size = *((uint32_t *) ip);
                ip += sizeof(uint32_t);

                printf("STRING ");
                for (uint32_t i = 0; i < string_size; i++)
                {
                    printf("%c", ip[i]);
                }

                bytes_consumed += 1 + sizeof(uint32_t) + string_size;
                break;

            case OPCODE_PRINT:
                printf("PRINT");
                bytes_consumed += 1;
                break;

            case OPCODE_CAST:
                ip += 1;

                uint8_t datatype_u8 = *ip;
                printf("CAST %s", _datatype_name((data_type_e) datatype_u8));
                bytes_consumed += 2;
                break;

            case OPCODE_JUMP:
                ip += 1;

                offset = *((uint32_t *) ip);
                printf("JUMP %d", offset);
                bytes_consumed += 1 + sizeof(int32_t);
                break;

            case OPCODE_JUMP_IF_FALSE:
                ip += 1;

                offset = *((uint32_t *) ip);
                printf("JUMP_IF_FALSE %d", offset);
                bytes_consumed += 1 + sizeof(int32_t);
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
        instructions_consumed += 1;
    }

    return DISASSEMBLE_OK;
}
