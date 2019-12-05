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

    vm_int_t int1val = 2;
    vm_float_t floatval = 16.0;
    vm_int_t int2val = 4;
    char *str1val = "Hello, ";
    char *str2val = "world!";
    vm_int_t int3val = 4;

    uint32_t backpatch_location;

    (void) bytecode_emit_define_const(&program, DATATYPE_INT, &int1val);
    (void) bytecode_emit_define_const(&program, DATATYPE_FLOAT, &floatval);
    (void) bytecode_emit_define_const(&program, DATATYPE_INT, &int2val);
    (void) bytecode_emit_define_const(&program, DATATYPE_STRING, str1val);
    (void) bytecode_emit_define_const(&program, DATATYPE_STRING, str2val);
    (void) bytecode_emit_define_const(&program, DATATYPE_INT, &int3val);
    (void) bytecode_emit_load_const(&program, 0);
    (void) bytecode_emit_load_const(&program, 1);
    (void) bytecode_emit_add(&program);
    (void) bytecode_emit_load_const(&program, 2);
    (void) bytecode_emit_div(&program);
    (void) bytecode_emit_cast(&program, DATATYPE_STRING, 10);
    (void) bytecode_emit_print(&program);
    (void) bytecode_emit_load_const(&program, 3);
    (void) bytecode_emit_load_const(&program, 4);
    (void) bytecode_emit_add(&program);

    (void) bytecode_emit_bool(&program, 1);
    (void) bytecode_emit_backpatched_jump_if_false(&program, &backpatch_location);
    (void) bytecode_emit_load_const(&program, 5);
    (void) bytecode_emit_mult(&program);
    (void) bytecode_backpatch_jump(&program, backpatch_location,
                                   program.used_bytes - backpatch_location);

    (void) bytecode_emit_print(&program);
    (void) bytecode_emit_end(&program);


    printf("\n-------- raw bytecode --------\n\n");
    bytecode_dump_raw(&program);
    printf("\n\n");

    printf("--------- disassembly --------\n\n");
    disassemble_bytecode(&program, 0, 0);
    printf("\n\n");

    vm_status_e vm_err;
    vm_instance_t ins;

    if ((vm_err = vm_verify(&program)) != VM_OK)
    {
        printf("vm_verify failed, status %d\n", vm_err);
        return vm_err;
    }

    if ((vm_err = vm_create(&ins)) != VM_OK)
    {
        printf("vm_create failed, status %d\n", vm_err);
        return vm_err;
    }

    printf("------- execution output ------\n\n");
    if ((vm_err = vm_execute(&ins, &program)) != VM_OK)

    {
        printf("vm_execute failed, status %d\n", vm_err);
        return vm_err;
    }

    printf("\n\n");
    bytecode_destroy(&program);
}
