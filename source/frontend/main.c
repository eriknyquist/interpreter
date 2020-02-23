#include <stdio.h>

#include "compiler_api.h"

int main(void)
{
    compiler_status_e err;
    if ((err = compiler_compile_file("testfile")) != COMPILER_OK)
    {
        printf("compiler_compile_file failed, status %d\n", err);
        return 1;
    }

    return 0;
}
