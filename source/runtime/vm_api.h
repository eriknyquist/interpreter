#ifndef VM_API_H_
#define VM_API_H_


#include "data_types.h"
#include "bytecode_api.h"
#include "runtime_error_api.h"


/* Enumeration of status codes returned by virtual machine functions */
typedef enum
{
    VM_OK,
    VM_MEMORY_ERROR,
    VM_INVALID_PARAM,
    VM_RUNTIME_ERROR,
    VM_ERROR
} vm_status_e;


/* Structure for data representing a VM instance */
typedef struct
{
    runtime_error_e runtime_error;
    callstack_t callstack;
} vm_instance_t;


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
