#ifndef RUNTIME_ERROR_H_
#define RUNTIME_ERROR_H_

typedef enum
{
    RUNTIME_ERROR_NONE,
    RUNTIME_ERROR_INVALID_OPCODE,
    RUNTIME_ERROR_MEMORY,
    RUNTIME_ERROR_ARITHMETIC,
    RUNTIME_ERROR_INTERNAL,
    NUM_RUNTIME_ERRORS
} runtime_error_e;


void runtime_error_set(runtime_error_e error);


runtime_error_e runtime_error_get(void);


#endif /* RUNTIME_ERROR_H_ */
