#include <stdio.h>

#include "vm_api.h"
#include "bytecode_api.h"
#include "disassemble_api.h"
#include "vm_api.h"


int main(void)
{
    bytecode_t program;

    bytecode_status_e bytecode_err = bytecode_create(&program);
    if (BYTECODE_OK != bytecode_err)
    {
        printf("Failed to create bytecode object, status %d", bytecode_err);
        return bytecode_err;
    }

    (void) bytecode_emit_int(&program, 3);
    (void) bytecode_emit_float(&program, 2.0);
    (void) bytecode_emit_add(&program);
    (void) bytecode_emit_int(&program, 2);
    (void) bytecode_emit_div(&program);
    (void) bytecode_emit_print(&program);
    (void) bytecode_emit_string(&program, "Hello, world!");
    (void) bytecode_emit_print(&program);
    (void) bytecode_emit_end(&program);

    bytecode_dump_raw(&program);

    disassemble_bytecode(&program, 0);

    vm_status_e vm_err;
    vm_instance_t ins;

    if ((vm_err = vm_create(&ins)) != VM_OK)
    {
        printf("vm_create failed, status %d\n", vm_err);
        return vm_err;
    }

    if ((vm_err = vm_execute(&ins, program.bytecode)) != VM_OK)
    {
        printf("vm_execute failed, status %d\n", vm_err);
        return vm_err;
    }

    bytecode_destroy(&program);
}
