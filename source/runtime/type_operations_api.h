#ifndef TYPE_OPERATIONS_API_H_
#define TYPE_OPERATIONS_API_H_

#include "data_types.h"


typedef enum
{
    TYPE_OK,
    TYPE_NO_CAST_REQUIRED,
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


/**
 * Creates a new object by casting the provided object to the specified data type
 *
 * @param    object      Pointer to the object to cast
 * @param    output      Pointer to the output object
 * @param    type        Data type to cast to
 * @param    extra_data  If casting from string to int, this value specifies the
 *                       numerical base that the source string is written in, from
 *                       2-36 (as specified by standard docs for strtol). if casting
 *                       from float to string, this value species to number of digits
 *                       after the decimal point to be included in the output string.
 *
 * @return   TYPE_OK if casting succeeded. If casting generated a runtime error,
 *           then TYPE_RUNTIME_ERROR will be returned, otherwise TYPE_INVALID_CAST
 *           if the source type cannot be converted to the requested type.
 */
type_status_e type_cast_to(object_t *object, object_t *output,
                           data_type_e type, uint16_t extra_data);


/**
 * Perform the specified arithmetic operation using the provided operands, and
 * create a new value containing the result
 *
 * @param    lhs         Pointer to LHS value for the operation
 * @param    rhs         Pointer to RHS value for the operation
 * @param    result      Pointer to result value
 * @param    arith_type  Arithmetic operation to perform
 *
 * @return   TYPE_OK if operation succeeded. If a runtime error was generated,
 *           then TYPE_RUNTIME_ERROR will be returned, otherwise TYPE_INVALID_ARITHMETIC
 *           if no function exists for handling the given LHS and RHS types
 */
type_status_e type_arithmetic(object_t *lhs, object_t *rhs, object_t *result,
                              arith_type_e arith_type);


#endif /* TYPE_OPERATIONS_API_H_ */
