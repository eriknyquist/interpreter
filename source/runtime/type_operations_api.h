#ifndef TYPE_OPERATIONS_API_H_
#define TYPE_OPERATIONS_API_H_

#include "data_types.h"


typedef enum
{
    TYPE_OK,
    TYPE_INVALID_PARAM,
    TYPE_INVALID_CAST,
    TYPE_INVALID_ARITHMETIC,
    TYPE_RUNTIME_ERROR,
    TYPE_ERROR
} type_status_e;


typedef enum
{
    ARITH_ADD,
    ARITH_SUB,
    ARITH_MULT,
    ARITH_DIV
} arith_type_e;


type_status_e type_cast_to(object_t *object, data_type_e type);


type_status_e type_arithmetic(object_t *lhs, object_t *rhs, object_t *result,
                              arith_type_e arith_type);


#endif /* TYPE_OPERATIONS_API_H_ */
