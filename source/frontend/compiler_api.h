#ifndef COMPILER_API_H
#define COMPILER_API_H

typedef enum
{
    COMPILER_OK,
    COMPILER_INVALID_PARAM,
    COMPILER_ERROR
} compiler_status_e;


compiler_status_e compiler_compile_file(const char *filename);

#endif /* COMPILER_API_H */
