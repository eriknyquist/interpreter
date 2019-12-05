#ifndef BYTECODE_COMMON_H
#define BYTECODE_COMMON_H


/* C type representing an encoded opcode */
typedef uint8_t opcode_t;


/* Structure representing a dynamically-sized chunk of bytecode */
typedef struct
{
    opcode_t *bytecode;    // Pointer to start of bytecode
    opcode_t *ip;          // Pointer to next instruction to be executed
    size_t total_bytes;    // Total bytes allocated for bytecode
    size_t used_bytes;     // Allocated bytes in use
} bytecode_t;


/* Enumeration of all valid opcode values */
typedef enum
{
    OPCODE_NOP,           // Do nothing
    OPCODE_ADD,           // Pop two values, add them, push result
    OPCODE_SUB,           // Pop two values, subtract one from the other, push result
    OPCODE_MULT,          // Pop two values, multiply them, push result
    OPCODE_DIV,           // Pop two values, divide ome by the other, push result
    OPCODE_INT,           // Create new immediate integer value and push
    OPCODE_FLOAT,         // Create new immediate float value and push
    OPCODE_STRING,        // Create new immediate string value and push
    OPCODE_BOOL,          // Create new immediate bool value and push
    OPCODE_PRINT,         // Pop a value and print it
    OPCODE_CAST,          // Pop a value, cast it to another type, push result
    OPCODE_JUMP,          // Jump to offset unconditionally
    OPCODE_JUMP_IF_FALSE, // Pop a value, cast to bool, jump to offset if false
    OPCODE_DEFINE_CONST,  // Add a new value to the constant pool
    OPCODE_LOAD_CONST,    // Load a value from constant pool and push
    OPCODE_END,           // Sentinel value indicating end of the program
    NUM_OPCODES
} opcode_e;


#endif /* BYTECODE_COMMON_H */
