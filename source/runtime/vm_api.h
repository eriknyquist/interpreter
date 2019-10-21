#ifndef VM_API_H_
#define VM_API_H_


#include "data_types.h"


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


/* Enumeration of status codes returned by virtual machine functions */
typedef enum
{
    VM_OK,
    VM_MEMORY_ERROR,
    VM_INVALID_PARAM,
    VM_ERROR
} vm_status_e;


/* Type of an encoded opcode */
typedef uint8_t opcode_t;


/* Structure for data representing a VM instance */
typedef struct
{
    callstack_t callstack;
} vm_instance_t;


#endif /* VM_API_H_ */
