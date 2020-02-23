#include <stdio.h>

#include "file_reader_api.h"
#include "scanner_api.h"
#include "compiler_api.h"

static void _show_error_from_token(const char *msg, file_in_memory_t *file, token_t *tok)
{
    // Find end of the line the token is on
    char *end = tok->lexeme + tok->lexeme_size;
    while (*end && ('\n' != *end))
    {
        end++;
    }

    // Find first character of the line the token is on
    char *start = tok->lexeme;
    while ((start != file->data) && ('\n' != *start))
    {
        start--;
    }

    // Error location will always be the last character of error token's lexeme
    unsigned colno = (tok->colno + tok->lexeme_size) - 2u;

    unsigned line_len = (unsigned) (end - start);

    printf("\n(%s, line %ld, column %u) %s\n\n", file->filename, tok->lineno,
                                                  colno, msg);

    printf("%.*s\n", (int) line_len, start);

    for (int i = 0; i < colno; i++)
    {
        printf(" ");
    }

    printf("^\n\n");
}

compiler_status_e compiler_compile_file(const char *filename)
{
    if (NULL == filename)
    {
        return COMPILER_INVALID_PARAM;
    }

    char *pos;
    token_t tok;
    file_reader_status_e file_err;
    file_in_memory_t file;

    if ((file_err = file_reader_load(&file, filename)) != FILE_READER_OK)
    {
        return COMPILER_ERROR;
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
            _show_error_from_token(scanner_error_message(), &file, &tok);
            break;
        }
    }

    if ((file_err = file_reader_destroy(&file)) != FILE_READER_OK)
    {
        return COMPILER_ERROR;
    }

    return COMPILER_OK;
}
