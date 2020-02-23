#ifndef SCANNER_API_H_
#define SCANNER_API_H_


#include <stdint.h>
#include <stdlib.h>


// Fixed non-alpha single-byte lexemes
#define TOKEN_LPAREN_LEXEME        '('
#define TOKEN_RPAREN_LEXEME        ')'
#define TOKEN_LBRACE_LEXEME        '{'
#define TOKEN_RBRACE_LEXEME        '}'
#define TOKEN_DOT_LEXEME           '.'
#define TOKEN_COMMA_LEXEME         ','
#define TOKEN_SQUOTE_LEXEME        '\''
#define TOKEN_DQUOTE_LEXEME        '"'
#define TOKEN_ASSIGN_LEXEME        '='
#define TOKEN_GREATER_LEXEME       '>'
#define TOKEN_LESS_LEXEME          '<'
#define TOKEN_NEGATE_LEXEME        '!'
#define TOKEN_ADD_LEXEME           '+'
#define TOKEN_SUB_LEXEME           '-'
#define TOKEN_DIV_LEXEME           '/'
#define TOKEN_MULT_LEXEME          '*'
#define TOKEN_MOD_LEXEME           '%'

// Fixed non-alpha multi-byte lexemes
#define TOKEN_CADD_LEXEME          "+="
#define TOKEN_CSUB_LEXEME          "-="
#define TOKEN_CDIV_LEXEME          "/="
#define TOKEN_CMULT_LEXEME         "*="
#define TOKEN_CMOD_LEXEME          "%="
#define TOKEN_EQUAL_LEXEME         "=="
#define TOKEN_NOTEQUAL_LEXEME      "!="
#define TOKEN_GREATEREQUAL_LEXEME  ">="
#define TOKEN_LESSEQUAL_LEXEME     "<="

// Fixed alpha multi-byte lexemes
#define TOKEN_IF_LEXEME            "if"
#define TOKEN_IN_LEXEME            "in"
#define TOKEN_WHILE_LEXEME         "while"
#define TOKEN_FOR_LEXEME           "for"
#define TOKEN_AND_LEXEME           "and"
#define TOKEN_OR_LEXEME            "or"
#define TOKEN_PRINT_LEXEME         "print"
#define TOKEN_TRUE_LEXEME          "true"
#define TOKEN_FALSE_LEXEME         "false"

// Don't forget to change this when adding/removing a fixed alpha token!
#define NUM_FIXED_ALPHA_TOKENS (9)


typedef enum
{
    TOKEN_LPAREN,               // (
    TOKEN_RPAREN,               // )
    TOKEN_LBRACE,               // {
    TOKEN_RBRACE,               // }
    TOKEN_DOT,                  // .
    TOKEN_COMMA,                // ,
    TOKEN_SQUOTE,               // '
    TOKEN_DQUOTE,               // "
    TOKEN_ASSIGN,               // =
    TOKEN_GREATER,              // >
    TOKEN_LESS,                 // <
    TOKEN_NEGATE,               // !
    TOKEN_ADD,                  // +
    TOKEN_SUB,                  // -
    TOKEN_DIV,                  // /
    TOKEN_MULT,                 // *
    TOKEN_MOD,                  // %
    TOKEN_CADD,                 // +=
    TOKEN_CSUB,                 // -=
    TOKEN_CDIV,                 // /=
    TOKEN_CMULT,                // *=
    TOKEN_CMOD,                 // %=
    TOKEN_EQUAL,                // ==
    TOKEN_NOTEQUAL,             // !=
    TOKEN_GREATEREQUAL,         // >=
    TOKEN_LESSEQUAL,            // <=
    TOKEN_IF,                   // if
    TOKEN_IN,                   // in
    TOKEN_WHILE,                // while
    TOKEN_FOR,                  // for
    TOKEN_AND,                  // and
    TOKEN_OR,                   // or
    TOKEN_PRINT,                // print
    TOKEN_TRUE,                 // true
    TOKEN_FALSE,                // false
    TOKEN_NAME,                 // function/variable names
    TOKEN_INT,                  // literal integer
    TOKEN_FLOAT,                // literal float
    TOKEN_ERROR,                // syntax error
    TOKEN_NONE,
    NUM_TOKENS
} token_type_e;


typedef struct
{
    char *lexeme;
    size_t lexeme_size;
    token_type_e token;
    uint64_t lineno;
    uint64_t colno;
} token_t;


typedef enum
{
    SCANNER_OK,
    SCANNER_INVALID_PARAM,
    SCANNER_ERROR
} scanner_status_e;


/**
 * Reset line and column counters-- should be called when changing to a new file
 */
void scanner_new_file(void);


/**
 * Consume bytes until a valid token is found
 *
 * @param   input   Pointer to string containing source code
 * @param   output  Pointer to location to store token
 *
 * @return  Pointer to first byte after found token, or NULL if no token was found
 */
char *scanner_scan_token(char *input, token_t *output);


/**
 * If the last token returned was of type TOKEN_ERROR, then this function
 * will return a human-readable message describing the error
 */
const char *scanner_error_message(void);


/**
 * Print a token_t object in a human-readable format
 */
void scanner_print_token(token_t *tok);

#endif /* SCANNER_API_H_ */
