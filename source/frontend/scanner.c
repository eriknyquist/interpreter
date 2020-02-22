#include <stdio.h>

#include "scanner_api.h"


// macro for testing if a byte mactches an ASCII whitespace character
#define IS_WHITESPACE(c) (c <= ' ')

// macro for testing if a byte matches an ASCII uppercase character
#define IS_UPPERCASE(c) ((c >= 'A') && (c <= 'Z'))

// macro for testing if a byte matches an ASCII lowercase character
#define IS_LOWERCASE(c) ((c >= 'a') && (c <= 'z'))

// macro for testing if a byte matches a valid ASCII character for object names
#define IS_NAME_CHAR(c) (IS_LOWERCASE(c) || IS_UPPERCASE(c) || (c == '_'))

// macro for testing if a byte matches an ASCII decimal character (0-9)
#define IS_DEC_DIGIT(c) ((c >= '0') && (c <= '9'))

// macro for testing if a byte matches an ASCII hex character (0-9, a-f, A-F)
#define IS_HEX_DIGIT(c) (((c >= 'a') && (c <= 'f')) || \
                         ((c >= 'A') && (c <= 'F')) || \
                         IS_DEC_DIGIT(c))


// macro for testing if a byte matches an ASCII decimal character (0-9) or '.'
#define IS_FLOAT_CHAR(c) (IS_DEC_DIGIT(c) || ('.' == c))


// helper macro to set the current token_t
#define RETURN_TOKEN(_tok, _lexemeptr, _lexemesize, _toktype) \
{                                                             \
    _tok->lexeme = (_lexemeptr);                              \
    _tok->lexeme_size = (_lexemesize);                        \
    _tok->token = (_toktype);                                 \
    _tok->lineno = _lineno;                                   \
                                                              \
    return (_lexemeptr) + (_lexemesize);                      \
}

static const char *_token_names[NUM_TOKENS] =
{
    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
    "TOKEN_LBRACE",
    "TOKEN_RBRACE",
    "TOKEN_DOT",
    "TOKEN_COMMA",
    "TOKEN_SQUOTE",
    "TOKEN_DQUOTE",
    "TOKEN_ASSIGN",
    "TOKEN_GREATER",
    "TOKEN_LESS",
    "TOKEN_NEGATE",
    "TOKEN_ADD",
    "TOKEN_SUB",
    "TOKEN_DIV",
    "TOKEN_MULT",
    "TOKEN_MOD",
    "TOKEN_CADD",
    "TOKEN_CSUB",
    "TOKEN_CDIV",
    "TOKEN_CMULT",
    "TOKEN_CMOD",
    "TOKEN_EQUAL",
    "TOKEN_NOTEQUAL",
    "TOKEN_GREATEREQUAL",
    "TOKEN_LESSEQUAL",
    "TOKEN_IF",
    "TOKEN_IN",
    "TOKEN_WHILE",
    "TOKEN_FOR",
    "TOKEN_AND",
    "TOKEN_OR",
    "TOKEN_PRINT",
    "TOKEN_TRUE",
    "TOKEN_FALSE",
    "TOKEN_NAME",
    "TOKEN_INT",
    "TOKEN_FLOAT",
    "TOKEN_ERROR"
};


/**
 * Array of pointers to keyword strings. Note that the order here must match
 * the order in token_type_e, since we use the _keywords array position to
 * determine the token type
 */
static const char *_keywords[NUM_FIXED_ALPHA_TOKENS] =
{
    TOKEN_IF_LEXEME,
    TOKEN_IN_LEXEME,
    TOKEN_WHILE_LEXEME,
    TOKEN_FOR_LEXEME,
    TOKEN_AND_LEXEME,
    TOKEN_OR_LEXEME,
    TOKEN_PRINT_LEXEME,
    TOKEN_TRUE_LEXEME,
    TOKEN_FALSE_LEXEME
};


static uint64_t _lineno = 1u;


/**
 * Check if 'input' points to a keyword. Returns a pointer to the character
 * after the last character in the keyword, or NULL if no keyword was found.
 * If return value is non-NULL, 'tokentype' will be set to the token type for
 * the keyword that was found.
 */
static char *_check_for_keyword(char *input, token_type_e *tokentype)
{
    const char *found_kw = NULL;

    for (unsigned k = 0u; k < NUM_FIXED_ALPHA_TOKENS; k++)
    {
        found_kw = _keywords[k];
        unsigned i;

        for (i = 0u; found_kw[i] && input[i]; i++)
        {
            if (found_kw[i] != input[i])
            {
                found_kw = NULL;
                break;
            }
        }

        if (NULL != found_kw)
        {
            // Convert keywords array index into token_type_e
            *tokentype = (token_type_e) k + TOKEN_IF;

            return input + i;
        }
    }

    return NULL;
}


void scanner_reset_line_number(void)
{
    _lineno = 1u;
}


char *scanner_scan_token(char *input, token_t *output)
{

    if ((NULL == input) || (NULL == output))
    {
        return NULL;
    }

    // Eat whitespace/invisible characters
    while (*input && IS_WHITESPACE(*input))
    {
        if (*input == '\n')
        {
            _lineno += 1u;
        }

        input++;
    }

    if (!*input)
    {
        RETURN_TOKEN(output, NULL, 0, TOKEN_NONE);
    }

    // Handle alpha tokens

    char *lexeme_start;
    unsigned lexeme_len;

    if (IS_NAME_CHAR(*input))
    {
        lexeme_start = input;
        while (IS_NAME_CHAR(*(input + 1)) || IS_DEC_DIGIT(*(input + 1)))
        {
            input++;
        }

        lexeme_len = ((unsigned) (input - lexeme_start)) + 1u;
        token_type_e tokentype;
        char *ret;

        ret = _check_for_keyword(lexeme_start, &tokentype);
        if (NULL == ret)
        {
            // lexeme is an object name
            RETURN_TOKEN(output, lexeme_start, lexeme_len, TOKEN_NAME);
        }
        else
        {
            // lexeme is a keyword
            RETURN_TOKEN(output, lexeme_start, lexeme_len, tokentype);
        }
    }

    // Handle literal integers and floats
    if (IS_DEC_DIGIT(*input))
    {
        token_type_e tokentype;
        lexeme_start = input;

        if ('x' == *(input + 1))
        {
            input += 1;
            tokentype = TOKEN_INT;

            // This is a literal integer in hexadecimal
            while (IS_HEX_DIGIT(*(input + 1)))
            {
                input++;
            }
        }
        else
        {
            // This is a literal float or decimal integer
            uint8_t saw_dot = 0u;
            while (IS_FLOAT_CHAR(*(input + 1)))
            {
                input++;
                if ('.' == *input)
                {
                    if (saw_dot)
                    {
                        // Mutiple dots in a float doesn't make sense
                        lexeme_len = ((unsigned) (input - lexeme_start)) + 1u;
                        RETURN_TOKEN(output, lexeme_start, lexeme_len, TOKEN_ERROR);
                    }
                    saw_dot = 1;
                }
            }

            tokentype = saw_dot ? TOKEN_FLOAT : TOKEN_INT;
        }

        if (IS_NAME_CHAR(*(input + 1)))
        {
            // Invalid character in literal int/float
            lexeme_len = ((unsigned) (input - lexeme_start)) + 2;
            RETURN_TOKEN(output, lexeme_start, lexeme_len, TOKEN_ERROR);
        }

        lexeme_len = ((unsigned) (input - lexeme_start)) + 1u;
        RETURN_TOKEN(output, lexeme_start, lexeme_len, tokentype);
    }

    // Handle fixed non-alphabetic tokens

    switch(*input)
    {
        case TOKEN_LPAREN_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_LPAREN);
            break;

        case TOKEN_RPAREN_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_RPAREN);
            break;

        case TOKEN_LBRACE_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_LBRACE);
            break;

        case TOKEN_RBRACE_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_RBRACE);
            break;

        case TOKEN_DOT_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_DOT);
            break;

        case TOKEN_COMMA_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_COMMA);
            break;

        case TOKEN_SQUOTE_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_SQUOTE);
            break;

        case TOKEN_DQUOTE_LEXEME:
            RETURN_TOKEN(output, input, 1, TOKEN_DQUOTE);
            break;

        case TOKEN_ADD_LEXEME:
            if (TOKEN_CADD_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_CADD);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_ADD);
            break;

        case TOKEN_SUB_LEXEME:
            if (TOKEN_CSUB_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_CSUB);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_SUB);
            break;

        case TOKEN_DIV_LEXEME:
            if (TOKEN_CDIV_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_CDIV);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_DIV);
            break;

        case TOKEN_MULT_LEXEME:
            if (TOKEN_CMULT_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_CMULT);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_MULT);
            break;

        case TOKEN_MOD_LEXEME:
            if (TOKEN_CMOD_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_CMOD);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_MOD);
            break;

        case TOKEN_ASSIGN_LEXEME:
            if (TOKEN_EQUAL_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_EQUAL);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_ASSIGN);
            break;

        case TOKEN_GREATER_LEXEME:
            if (TOKEN_GREATEREQUAL_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_GREATEREQUAL);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_GREATER);
            break;

        case TOKEN_LESS_LEXEME:
            if (TOKEN_LESSEQUAL_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_LESSEQUAL);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_LESS);
            break;

        case TOKEN_NEGATE_LEXEME:
            if (TOKEN_NOTEQUAL_LEXEME[1] == input[1])
            {
                RETURN_TOKEN(output, input, 2, TOKEN_NOTEQUAL);
            }

            RETURN_TOKEN(output, input, 1, TOKEN_NEGATE);
            break;
    }

    RETURN_TOKEN(output, input, 1, TOKEN_ERROR);
}


void scanner_print_token(token_t *tok)
{
    if (tok->token >= NUM_TOKENS)
    {
        return;
    }

    printf("Token(type=%s, lexeme='%.*s', lineno=%lu)\n",
           _token_names[tok->token],
           (int) tok->lexeme_size,
           tok->lexeme,
           (unsigned long)tok->lineno);
}


