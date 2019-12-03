#ifndef VM_API_H_
#define VM_API_H_


#include "ulist_api.h"
#include "data_types.h"
#include "bytecode_api.h"
#include "runtime_common.h"


/* Enumeration of status codes returned by virtual machine functions */
typedef enum
{
    VM_OK,
    VM_MEMORY_ERROR,
    VM_INVALID_PARAM,
    VM_INVALID_OPCODE,
    VM_RUNTIME_ERROR,
    VM_ERROR
} vm_status_e;


/**
 * Initializes a virtual machine instance; allocates memory for the first
 * stack frame and sets up the data stack for that frame.
 *
 * @param    instance    Pointer to VM instance to initialize
 *
 * @return   VM_OK if initialization was successful
 */
vm_status_e vm_create(vm_instance_t *instance);


/**
 * Verify the provided bytecode contains only valid opcodes. Should be called
 * before executing a chunk of bytecode; avoids requiring vm_execute to check
 * each opcode before executing.
 *
 * @param    bytecode    Pointer to bytecode
 *
 * @return   VM_OK if bytecode was verified successfully, VM_INVALID_OPCODE otherwise
 */
vm_status_e vm_verify(opcode_t *bytecode, size_t max_bytes);


/**
 * Tears down an initialized VM instance and deallocates all memory, including
 * memory used for all call stack frames and data stacks
 *
 * @param    instance    Pointer to VM instance to destroy
 *
 * @return   VM_OK if teardown was successful
 */
vm_status_e vm_destroy(vm_instance_t *instance);


/**
 * Executes a chunk of bytecode
 *
 * @param    instance    Pointer to VM instance to destroy
 * @param    bytecode    Pointer to bytecode
 *
 * @return   VM_OK if execution completed successfully
 */
vm_status_e vm_execute(vm_instance_t *instance, opcode_t *bytecode);


#endif /* VM_API_H_ */
