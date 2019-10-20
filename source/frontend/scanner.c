#include "scanner_api.h"


#define SET_TOKEN(_tok, _lexemeptr, _lexemesize, _toktype)  \
                do {                           \
                    _tok->lexeme = (_lexemeptr);    \
                    _tok->lexeme_size = (_lexemesize); \
                    _tok->token = (_toktype);    \
                    _tok->lineno = _lineno;     \
                } while(0)


static uint64_t _lineno = 1u;


void scanner_reset_line_number(void)
{
    _lineno = 1u;
}


scanner_status_e scanner_scan(char *input, size_t max, token_t *output)
{

    if ((NULL == input) || (NULL == output) || (max == 0u))
    {
        return SCANNER_INVALID_PARAM;
    }

    for (size_t i = 0; i < max; i++)
    {
        // Eat whitespace/invisible characters
        if((unsigned) input[i] < (unsigned) ' ')
        {
            if (*input == '\n')
            {
                _lineno += 1u;
            }

            continue;
        }


    }

    SET_TOKEN(output, NULL, 0, TOKEN_NONE);
    return SCANNER_OK;
}
