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

        case DATATYPE_BOOL:
            return "BOOL";
            break;

        default:
            return "????";
            break;
    }

    return "????";
}


size_t _print_encoded_data(void *data, int *chars_printed)
{
    size_t bytes_consumed;
    uint8_t *bytes = data;

    data_type_e datatype = (data_type_e) *bytes;
    bytes_consumed = 1u;
    bytes += 1;

    switch (datatype)
    {
        case DATATYPE_INT:
            *chars_printed += printf("%d", *((vm_int_t *) bytes));
            bytes_consumed += sizeof(vm_int_t);
            break;

        case DATATYPE_FLOAT:
            *chars_printed += printf("%.2f", *((vm_float_t *) bytes));
            bytes_consumed += sizeof(vm_float_t);
            break;

        case DATATYPE_BOOL:
            *chars_printed += printf("%s", (*((vm_bool_t *) bytes)) ? "True" : "False");
            bytes_consumed += sizeof(vm_bool_t);
            break;

        case DATATYPE_STRING:
        {
            uint32_t string_size = *((uint32_t *) bytes);
            bytes += sizeof(uint32_t);

            *chars_printed += printf("%.*s", string_size, bytes);
            bytes_consumed += sizeof(uint32_t) + string_size;
            break;
        }
        default:
            return 0u;
    }

    return bytes_consumed;
}


/**
 * @see disassemble_api.h
 */
disassemble_status_e disassemble_bytecode(bytecode_t *program, size_t offset_bytes,
                                          size_t num_instructions)
{
    if (NULL == program)
    {
        return DISASSEMBLE_INVALID_PARAM;
    }

    uint8_t *ip;
    int32_t offset;
    size_t bytes_consumed = offset_bytes;
    size_t instructions_consumed = 0;

    while (bytes_consumed < program->used_bytes)
    {

        if (num_instructions && (instructions_consumed >= num_instructions))
        {
            break;
        }

        int chars_printed = 0u;
        uint32_t bytes_before = bytes_consumed;
        ip = program->bytecode + bytes_consumed;
        chars_printed += printf("%08lx ", bytes_consumed);

        switch ((opcode_e)  *ip)
        {
            case OPCODE_NOP:
                chars_printed += printf("NOP");
                bytes_consumed += 1;
                break;

            case OPCODE_ADD:
                chars_printed += printf("ADD");
                bytes_consumed += 1;
                break;

            case OPCODE_SUB:
                chars_printed += printf("SUB");
                bytes_consumed += 1;
                break;

            case OPCODE_MULT:
                chars_printed += printf("MULT");
                bytes_consumed += 1;
                break;

            case OPCODE_DIV:
                chars_printed += printf("DIV");
                bytes_consumed += 1;
                break;

            case OPCODE_INT:
                chars_printed += printf("INT %d", *((vm_int_t *) (ip + 1)));
                bytes_consumed += sizeof(vm_int_t) + 1;
                break;

            case OPCODE_BOOL:
                chars_printed += printf("BOOL %s", *((vm_bool_t *) (ip + 1)) ? "True" : "False");
                bytes_consumed += sizeof(vm_bool_t) + 1;
                break;

            case OPCODE_FLOAT:
                chars_printed += printf("FLOAT %.4f", *((vm_float_t *) (ip + 1)));
                bytes_consumed += sizeof(vm_float_t) + 1;
                break;

            case OPCODE_STRING:
                ip += 1;

                uint32_t string_size = *((uint32_t *) ip);
                ip += sizeof(uint32_t);

                chars_printed += printf("STRING ");
                for (uint32_t i = 0; i < string_size; i++)
                {
                    chars_printed += printf("%c", ip[i]);
                }

                bytes_consumed += 1 + sizeof(uint32_t) + string_size;
                break;

            case OPCODE_PRINT:
                chars_printed += printf("PRINT");
                bytes_consumed += 1;
                break;

            case OPCODE_CAST:
                ip += 1;

                uint8_t datatype_u8 = *ip;
                ip += 1;

                uint16_t extra_data = *((uint16_t *) ip);

                chars_printed += printf("CAST %s %d", _datatype_name((data_type_e) datatype_u8),
                                     extra_data);
                bytes_consumed += 1 + sizeof(uint8_t) + sizeof(uint16_t);
                break;

            case OPCODE_JUMP:
                ip += 1;

                offset = *((uint32_t *) ip);
                chars_printed += printf("JUMP %x", offset);
                bytes_consumed += 1 + sizeof(int32_t);
                break;

            case OPCODE_JUMP_IF_FALSE:
                ip += 1;

                offset = *((uint32_t *) ip);
                chars_printed += printf("JUMP_IF_FALSE %x", offset);
                bytes_consumed += 1 + sizeof(int32_t);
                break;

            case OPCODE_LOAD_CONST:
            {
                ip += 1;
                uint32_t index = *((uint32_t *) ip);
                chars_printed += printf("LOAD_CONST %d", index);
                bytes_consumed += 1 + sizeof(uint32_t);
                break;
            }
            case OPCODE_DEFINE_CONST:
            {
                ip += 1;
                chars_printed += printf("DEFINE_CONST ");
                size_t consumed = _print_encoded_data(ip, &chars_printed);
                if (0u == consumed)
                {
                    return DISASSEMBLE_ERROR;
                }

                bytes_consumed += 1 + consumed;
                break;
            } 

            case OPCODE_END:
                chars_printed += printf("END");
                bytes_consumed += 1;
                break;

            default:
                chars_printed += printf("????");
                bytes_consumed += 1;
                break;
        }

        printf("%*s", 50 - chars_printed, "(");
        for (uint32_t i = bytes_before; i < bytes_consumed; i++)
        {
            printf("%02x", *(program->bytecode + i));
            if (i < (bytes_consumed - 1))
            {
                printf(" ");
            }
        }
        chars_printed += printf(")\n");
        instructions_consumed += 1;
    }

    return DISASSEMBLE_OK;
}
