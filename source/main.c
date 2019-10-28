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
    (void) bytecode_emit_cast(&program, DATATYPE_STRING);
    (void) bytecode_emit_cast(&program, DATATYPE_INT);
    (void) bytecode_emit_print(&program);
    (void) bytecode_emit_string(&program, "Hello, ");
    (void) bytecode_emit_string(&program, "world!");
    (void) bytecode_emit_add(&program);
    (void) bytecode_emit_print(&program);
    (void) bytecode_emit_end(&program);


    printf("\n-------- raw bytecode --------\n\n");
    bytecode_dump_raw(&program);
    printf("\n\n");

    printf("--------- disassembly --------\n\n");
    disassemble_bytecode(&program, 0);
    printf("\n\n");

    vm_status_e vm_err;
    vm_instance_t ins;

    if ((vm_err = vm_create(&ins)) != VM_OK)
    {
        printf("vm_create failed, status %d\n", vm_err);
        return vm_err;
    }

    printf("------- execution output ------\n\n");
    if ((vm_err = vm_execute(&ins, program.bytecode)) != VM_OK)
    {
        printf("vm_execute failed, status %d\n", vm_err);
        return vm_err;
    }

    printf("\n\n");
    bytecode_destroy(&program);
}
