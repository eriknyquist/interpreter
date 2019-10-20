#ifndef SCANNER_API_H_
#define SCANNER_API_H_


#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    TOKEN_LPAREN,               // (
    TOKEN_RPAREN,               // )
    TOKEN_ASSIGN,               // =
    TOKEN_GREATER,              // >
    TOKEN_LESS,                 // <
    TOKEN_EQUAL,                // ==
    TOKEN_NOTEQUAL,             // !=
    TOKEN_GREATEREQUAL,         // >=
    TOKEN_LESSEQUAL,            // <=
    TOKEN_NAME,                 // function/variable names
    TOKEN_NONE
} token_type_e;


typedef struct
{
    char *lexeme;
    size_t lexeme_size;
    token_type_e token;
    uint64_t lineno;
} token_t;


typedef enum
{
    SCANNER_OK,
    SCANNER_INVALID_PARAM,
    SCANNER_ERROR
} scanner_status_e;


void scanner_reset_line_number(void);

scanner_status_e scanner_scan(char *input, size_t max_input, token_t *output);


#endif /* SCANNER_API_H_ */
