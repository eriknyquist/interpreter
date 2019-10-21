#include <stdio.h>

#include "bytecode_api.h"
#include "disassemble_api.h"


int main(void)
{
    bytecode_t program;

    bytecode_status_e bytecode_err = bytecode_create(&program);
    if (BYTECODE_OK != bytecode_err)
    {
        printf("Failed to create bytecode object, status %d", bytecode_err);
        return bytecode_err;
    }

    (void) bytecode_emit_int(&program, 26);
    (void) bytecode_emit_float(&program, 17.298);
    (void) bytecode_emit_add(&program);
    (void) bytecode_emit_print(&program);

    bytecode_dump_raw(&program);

    disassemble_bytecode(&program);
}
