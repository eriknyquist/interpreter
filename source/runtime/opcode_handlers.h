#ifndef OPCODE_HANDLERS_H_
#define OPCODE_HANDLERS_H_


#include "data_types.h"
#include "bytecode_api.h"
#include "type_operations_api.h"
#include "runtime_error_api.h"
#include "runtime_common.h"


/**
 * Structure representing all object types that can be pushed to the data stack
 */
typedef struct
{
    union
    {
        object_t object;
        data_object_t data_object;
    } payload;
} data_stack_entry_t;


/* Handler function for opcodes */
typedef opcode_t *(*op_handler_t)(opcode_t *, vm_instance_t *);



opcode_t *opcode_handler_nop(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_add(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_sub(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_mult(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_div(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_int(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_float(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_string(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_bool(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_print(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_cast(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_jump(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_jump_if_false(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_define_const(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_load_const(opcode_t *opcode, vm_instance_t *instance);


opcode_t *opcode_handler_end(opcode_t *opcode, vm_instance_t *instance);


#endif /* OPCODE_HANDLERS_H_ */
