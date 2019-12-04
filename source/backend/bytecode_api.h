/**
 * Functions for generating bytecode
 */

#ifndef BYTECODE_API_H_
#define BYTECODE_API_H_

#include <stdio.h>
#include <stdint.h>

#include "data_types.h"
#include "bytecode_common.h"


/* Error codes returned by bytecode generation functions */
typedef enum
{
    BYTECODE_OK,                 // Operation completed successfully
    BYTECODE_INVALID_PARAM,      // Invalid parameter passed to function
    BYTECODE_INVALID_BACKPATCH,  // Position value points to instruction that can't be backpatched
    BYTECODE_MEMORY_ERROR,       // Failed to allocate memory
    BYTECODE_ERROR               // Unspecified internal error
} bytecode_status_e;


/**
 * Initialize a new bytecode_t instance to hold a chunk of bytecode.
 * Allocated size will be increased as needed (using malloc/realloc) as
 * bytecode is added.
 *
 * @param    program   Pointer to bytecode_t instance to initialize
 *
 * @return   BYTECODE_OK if initialization was successful
 */
bytecode_status_e bytecode_create(bytecode_t *program);


/**
 * De-allocate any memory associated with an initialized bytecode_t instance
 *
 * @param    program   Pointer to bytecode_t instance to de-allocate
 *
 * @return   BYTECODE_OK if de-allocation was successful
 */
bytecode_status_e bytecode_destroy(bytecode_t *program);


/**
 * Dump raw bytecode to stdout as ASCII hex characters
 *
 * @param    program   Pointer to bytecode_t instance to dump
 */
void bytecode_dump_raw(bytecode_t *program);


/**
 * Add INT instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    value     Integer value
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_int(bytecode_t *program, vm_int_t value);


/**
 * Add FLOAT instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    value     Float value
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_float(bytecode_t *program, vm_float_t value);


/**
 * Add STRING instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    value     Pointer to string data (must be null-terminated)
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_string(bytecode_t *program, char *value);


/**
 * Add BOOL instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    value     Boolean value (1=true, 0=false)
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_bool(bytecode_t *program, vm_bool_t value);


/**
 * Add CAST instruction to a bytecode chunk
 *
 * @param    program    Pointer to bytecode_t instance
 * @param    data_type  Data type to cast to
 * @param    data       Optional extra data. Used for the numerical base if
 *                      converting from string to int, or used as the number
 *                      of digits after the decimal point to be included when
 *                      converting from float to string
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_cast(bytecode_t *program, data_type_e data_type,
                                     uint16_t data);

/**
 * Add JUMP instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    offset    Offset to jump to, in bytes, relative to current position
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_jump(bytecode_t *program, int32_t offset);


/**
 * Add JUMP_IF_FALSE instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    offset    Offset to jump to, in bytes, relative to current position
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_jump_if_false(bytecode_t *program, int32_t offset);


/**
 * Add JUMP instruction to a bytecode chunk, but omit the offset value, to be
 * filled in later.
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    position  Pointer to location to store the position of the instruction
 *                     that is being added. This should be saved by the caller so
 *                     it can be used later to backpatch the offset value, when
 *                     the offset value is available
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_backpatched_jump(bytecode_t *program,
                                                 uint32_t *position);


/**
 * Add JUMP_IF_FALSE instruction to a bytecode chunk, but omit the offset value,
 * to be filled in later.
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    position  Pointer to location to store the position of the instruction
 *                     that is being added. This should be saved by the caller so
 *                     it can be used later to backpatch the offset value, when
 *                     the offset value is available
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_backpatched_jump_if_false(bytecode_t *program,
                                                          uint32_t *position);


/**
 * Backpatch a jump instruction that was previously added without an offset value
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    position  Position value that was populated when backpatched instruction
 *                     was emitted
 * @param    offset    Offset value to be patched in
 */
bytecode_status_e bytecode_backpatch_jump(bytecode_t *program, uint32_t position,
                                          int32_t offset);


/**
 * Add ADD instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_add(bytecode_t *program);


/**
 * Add SUB instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_sub(bytecode_t *program);


/**
 * Add MULT instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_mult(bytecode_t *program);


/**
 * Add DIV instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_div(bytecode_t *program);


/**
 * Add PRINT instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_print(bytecode_t *program);


/**
 * Add DEFINE_CONST instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    datatype  Data type of data being passed
 * @param    data      Pointer to data to encode
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_define_const(bytecode_t *program,
                                             data_type_e datatype, void *data);


/**
 * Add LOAD_CONST instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 * @param    index     Constant pool index to load
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_load_const(bytecode_t *program, uint32_t index);


/**
 * Add END instruction to a bytecode chunk
 *
 * @param    program   Pointer to bytecode_t instance
 *
 * @return   BYTECODE_OK if instruction was addedd successfuly
 */
bytecode_status_e bytecode_emit_end(bytecode_t *program);


#endif /* BYTECODE_API_H_ */
