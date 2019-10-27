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


vm_status_e vm_create(vm_instance_t *instance);


vm_status_e vm_execute(vm_instance_t *instance, opcode_t *bytecode);


#endif /* VM_API_H_ */
