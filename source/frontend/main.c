#include <stdio.h>

#include "file_reader_api.h"
#include "scanner_api.h"

int main(void)
{
    char *pos;
    token_t tok;
    file_reader_status_e file_err;
    file_in_memory_t file;

    if ((file_err = file_reader_load(&file, "testfile")) != FILE_READER_OK)
    {
        printf("file_reader_open failed, status %d\n", file_err);
        return 1;
    }

    pos = file.data;
    while (*pos)
    {
        pos = scanner_scan_token(pos, &tok);
        if (TOKEN_NONE == tok.token)
        {
            break;
        }

        scanner_print_token(&tok);

        if (TOKEN_ERROR == tok.token)
        {
            break;
        }
    }

    if ((file_err = file_reader_destroy(&file)) != FILE_READER_OK)
    {
        printf("file_reader_destroy failed, status %d\n", file_err);
        return 1;
    }
}
