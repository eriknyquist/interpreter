#include <stdlib.h>
#include <string.h>
#include "type_operations_api.h"
#include "opcode_handlers.h"
#include "string_cache_api.h"
#include "memory_manager_api.h"


/* Size allocated for string data in a DATATYPE_STRING object created to
 * hold the string-ified representation of an integer */
#define MAX_STRING_NUM_SIZE (32u)


/* Highest values allowed for number of decimal places when converting
 * a float object to a string object */
#define MAX_FLOAT_PLACES (32)


/* Size allocated for string data when coverting a bool to a string object */
#define BOOL_STRING_SIZE (6)


/* Strings used when converting bools to string objects */
#define BOOL_STRING_TRUE   "true"
#define BOOL_STRING_FALSE  "false"


/* Min/max valid values for base when converting a string object to an int
 * object, as specified by http://man7.org/linux/man-pages/man3/strtol.3.html */
#define MIN_STRING_INT_BASE (2)
#define MAX_STRING_INT_BASE (36)


#define TYPE_OPS_DEF(cast_int, cast_float, cast_string, cast_bool,                \
                     arith_int, arith_float, arith_string, arith_bool)            \
    {.cast_functions={(cast_int), (cast_float), (cast_string), (cast_bool)},      \
     .arith_functions={(arith_int), (arith_float), (arith_string), (arith_bool)}} \


/* Function for casting one data type to another type, creating a new object */
typedef type_status_e (*cast_func_t) (object_t *, object_t *, uint16_t);

/* Function for performing an arithmetic operation using two objects as operands,
 * and creating a new object containing the result */
typedef type_status_e (*arith_func_t) (object_t *, object_t *, object_t *,
                                       arith_type_e);

/**
 * Structure to hold function pointers for handling casting and arithmetic
 * operations for a single data type
 */
typedef struct
{
    cast_func_t cast_functions[NUM_DATATYPES];
    arith_func_t arith_functions[NUM_DATATYPES];
} type_operations_t;


/* Casting functions for ints */
static type_status_e _int_to_float(object_t *, object_t *, uint16_t);     /* Cast int to float */
static type_status_e _int_to_string(object_t *, object_t *, uint16_t);    /* Cast int to string */
static type_status_e _int_to_bool(object_t *, object_t *, uint16_t);      /* Cast int to bool */

/* Casting functions for floats */
static type_status_e _float_to_int(object_t *, object_t *, uint16_t);     /* Cast float to int */
static type_status_e _float_to_string(object_t *, object_t *, uint16_t);  /* Cast float to string */
static type_status_e _float_to_bool(object_t *, object_t *, uint16_t);    /* Cast float to bool */

/* Casting functions for strings */
static type_status_e _string_to_int(object_t *, object_t *, uint16_t);    /* Cast string to int */
static type_status_e _string_to_float(object_t *, object_t *, uint16_t);  /* Cast string to float */
static type_status_e _string_to_bool(object_t *, object_t *, uint16_t);   /* Cast string to bool */

/* Casting functions for bools */
static type_status_e _bool_to_int(object_t *, object_t *, uint16_t);      /* Cast bool to int */
static type_status_e _bool_to_float(object_t *, object_t *, uint16_t);    /* Cast bool to float */
static type_status_e _bool_to_string(object_t *, object_t *, uint16_t);   /* Cast bool to string */


/* Arithmetic when LHS is an int */
static type_status_e _arith_int_int(object_t *, object_t *, object_t *, arith_type_e);
static type_status_e _arith_int_float(object_t *, object_t *, object_t *, arith_type_e);
static type_status_e _arith_int_string(object_t *, object_t *, object_t *, arith_type_e);
/* static type_status_e _arith_int_bool(object_t *, object_t *, object_t *, arith_type_e);       (TODO) */


/* Arithmetic when LHS is a float */
static type_status_e _arith_float_int(object_t *, object_t *, object_t *, arith_type_e);
static type_status_e _arith_float_float(object_t *, object_t *, object_t *, arith_type_e);
/* static type_status_e _arith_float_string(object_t *, object_t *, object_t *, arith_type_e);   (TODO) */
/* static type_status_e _arith_float_bool(object_t *, object_t *, object_t *, arith_type_e);     (TODO) */


/* Arithmetic when LHS is a string */
static type_status_e _arith_string_int(object_t *, object_t *, object_t *, arith_type_e);
/* static type_status_e _arith_string_float(object_t *, object_t *, object_t *, arith_type_e);   (TODO) */
static type_status_e _arith_string_string(object_t *, object_t *, object_t *, arith_type_e);


/* Arithmetic when LHS is a bool */
static type_status_e _arith_bool_int(object_t *, object_t *, object_t *, arith_type_e);
static type_status_e _arith_bool_float(object_t *, object_t *, object_t *, arith_type_e);
/* static type_status_e _arith_bool_bool(object_t *, object_t *, object_t *, arith_type_e);      (TODO) */


/**
 * This table holds all function pointers required for casting or performing
 * arithmetic with all data types. The table is arranged such that the data types
 * of the LHS and RHS operands can be used to index the table and obtain the right
 * function for two operands of any given data type.
 *
 * Example:
 *
 * consider the following bytecode that approximates the expression (3 + 5.0):
 *
 *    INT 3       // push integer with value "3" to the stack
 *    FLOAT 5.0   // push float with value "5.0" to the stack
 *    ADD         // pop both values, add them, push result to the stack
 *
 * In this situation, 3 is the LHS of the operation (because it's on the left
 * side of the operator in the infix notation given above), and 5.0 is the RHS.
 *
 * In order to obtain the correct function for performing arithmetic with an int
 * as the LHS and a float as the RHS, we first index into the array using the LHS
 * type (int) to get the type_operations_t that contains function pointers for
 * arithmetic with an int as the LHS. Finally, we use the type of the RHS (float)
 * to index into the .arith_functions member of the retrieved type_operations_t
 * structure to get a pointer to the function that will perform the desired operation
 */
static type_operations_t _type_ops[NUM_DATATYPES] =
{
    TYPE_OPS_DEF(NULL, _int_to_float, _int_to_string, _int_to_bool,
                 _arith_int_int, _arith_int_float, _arith_int_string, NULL), // DATATYPE_INT

    TYPE_OPS_DEF(_float_to_int, NULL, _float_to_string, _float_to_bool,
                 _arith_float_int, _arith_float_float, NULL, NULL),          // DATATYPE_FLOAT

    TYPE_OPS_DEF(_string_to_int, _string_to_float, NULL, _string_to_bool,
                 _arith_string_int, NULL, _arith_string_string, NULL),       // DATATYPE_STRING

    TYPE_OPS_DEF(_bool_to_int, _bool_to_float, _bool_to_string, NULL,
                 _arith_bool_int, _arith_bool_float, NULL, NULL)             // DATATYPE_BOOL
};


static type_status_e _multiply_string(vm_int_t int_value, byte_string_t *string_value,
                                      byte_string_t *result_string)
{
    /* Ensure result string has enough space to hold multiple copies of result
     * string data, and we'll drop the null byte on all but the last */
    size_t result_string_size = ((string_value->size - 1) * int_value) + 1;

    char *temp_string;

    if ((temp_string = memory_manager_alloc(result_string_size)) == NULL)
    {
        RUNTIME_ERR(RUNTIME_ERROR_MEMORY, "memory_manager_alloc failed");
        return TYPE_RUNTIME_ERROR;
    }

    // Build new string by making multiple copies of the source string
    for (vm_int_t i = 0; i < int_value; i++)
    {
        char *target = temp_string + (i * (string_value->size - 1));
        (void) memcpy(target, string_value->bytes, string_value->size - 1);
    }


    string_cache_status_e cache_err;
    byte_string_t *new_byte_string;

    cache_err = string_cache_add(temp_string, result_string_size - 1, &new_byte_string);
    if (STRING_CACHE_OK != cache_err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "string_cache_add failed, status %d", cache_err);
        return TYPE_RUNTIME_ERROR;
    }

    memory_manager_free(temp_string);
    memcpy(result_string, new_byte_string, sizeof(byte_string_t));
    return TYPE_OK;
}


/* Casting functions */
static type_status_e _int_to_float(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_FLOAT;
    data_out->payload.float_value = (vm_float_t) data_obj->payload.int_value;
    return TYPE_OK;
}


static type_status_e _int_to_string(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    vm_int_t int_value = data_obj->payload.int_value;
    byte_string_t *new_byte_string;
    char temp_string[MAX_STRING_NUM_SIZE];

    int printed = snprintf(temp_string, MAX_STRING_NUM_SIZE, "%d", int_value);

    string_cache_status_e cache_err = string_cache_add(temp_string, printed + 1, &new_byte_string);
    if (STRING_CACHE_OK != cache_err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "string_cache_add failed, status %d", cache_err);
        return TYPE_RUNTIME_ERROR;
    }

    memcpy(&data_out->payload.string_value, new_byte_string, sizeof(byte_string_t));
    data_out->data_type = DATATYPE_STRING;
    return TYPE_OK;
}


static type_status_e _int_to_bool(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_BOOL;
    data_out->payload.bool_value = (data_obj->payload.int_value == 0) ? 0u : 1u;
    return TYPE_OK;
}



static type_status_e _string_to_int(object_t *object, object_t *output, uint16_t base)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    long int longval;
    char *endptr;

    if ((MIN_STRING_INT_BASE > base) || (MAX_STRING_INT_BASE < base))
    {
        RUNTIME_ERR(RUNTIME_ERROR_CAST, "base must be between %d-%d",
                                        MIN_STRING_INT_BASE, MAX_STRING_INT_BASE);
        return TYPE_RUNTIME_ERROR;
    }

    longval = strtol((char *) data_obj->payload.string_value.bytes, &endptr, (int) base);
    if (('\0' != *endptr) && ('.' != *endptr))
    {
        /* If endptr is not pointing at the null termination byte, then not all
         * characters in the string are valid */
        RUNTIME_ERR(RUNTIME_ERROR_CAST,
                    "Can't convert string '%s' to int",
                    data_obj->payload.string_value.bytes);
        return TYPE_RUNTIME_ERROR;
    }

    data_out->data_type = DATATYPE_INT;
    data_out->payload.int_value = (vm_int_t) longval;

    return TYPE_OK;
}


static type_status_e _string_to_float(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    double doubleval;
    char *endptr;

    doubleval = strtod((char *) data_obj->payload.string_value.bytes, &endptr);
    if ('\0' != *endptr)
    {
        /* If endptr is not pointing at the null termination byte, then not all
         * characters in the string are valid */
        RUNTIME_ERR(RUNTIME_ERROR_CAST,
                    "Can't convert string '%s' to float",
                    data_obj->payload.string_value.bytes);
        return TYPE_RUNTIME_ERROR;
    }

    data_out->data_type = DATATYPE_FLOAT;
    data_out->payload.float_value = (vm_float_t) doubleval;

    return TYPE_OK;
}


static type_status_e _string_to_bool(object_t *object, object_t *output, uint16_t base)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_BOOL;
    data_out->payload.bool_value = (data_obj->payload.string_value.size > 0u) ? 1u : 0u;
    return TYPE_OK;
}


static type_status_e _float_to_int(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_INT;
    data_out->payload.int_value = (vm_int_t) data_obj->payload.float_value;
    return TYPE_OK;
}


static type_status_e _float_to_string(object_t *object, object_t *output, uint16_t places)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    char *temp_string;
    byte_string_t *new_byte_string;
    size_t string_size = MAX_STRING_NUM_SIZE + places;

    if (MAX_FLOAT_PLACES < places)
    {
        RUNTIME_ERR(RUNTIME_ERROR_CAST,
                    "decimal places must be between 0-%d\n", MAX_FLOAT_PLACES);
        return TYPE_RUNTIME_ERROR;
    }

    if ((temp_string = memory_manager_alloc(string_size)) == NULL)
    {
        RUNTIME_ERR(RUNTIME_ERROR_MEMORY, "memory_manager_alloc failed");
        return TYPE_RUNTIME_ERROR;
    }

    // Build format string with requested number of places
    char fmt_string[16];
    if (snprintf(fmt_string, sizeof(fmt_string), "%%.%df", places) >= sizeof(fmt_string))
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL, "snprintf output truncated\n");
        return TYPE_RUNTIME_ERROR;
    }

    int printed = snprintf(temp_string, string_size, fmt_string,
                           data_obj->payload.float_value);

    string_cache_status_e cache_err = string_cache_add(temp_string, printed + 1, &new_byte_string);
    if (STRING_CACHE_OK != cache_err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "string_cache_add failed, status %d", cache_err);
        return TYPE_RUNTIME_ERROR;
    }

    memory_manager_free(temp_string);
    memcpy(&data_out->payload.string_value, new_byte_string, sizeof(byte_string_t));
    data_out->data_type = DATATYPE_STRING;
    return TYPE_OK;
}


static type_status_e _float_to_bool(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_BOOL;
    data_out->payload.bool_value = (data_obj->payload.float_value == 0.0) ? 0u : 1u;
    return TYPE_OK;
}


static type_status_e _bool_to_int(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_INT;
    data_out->payload.int_value = (vm_int_t) data_obj->payload.bool_value;
    return TYPE_OK;
}


static type_status_e _bool_to_float(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    data_out->data_type = DATATYPE_FLOAT;
    data_out->payload.float_value = (vm_float_t) data_obj->payload.bool_value;
    return TYPE_OK;
}


static type_status_e _bool_to_string(object_t *object, object_t *output, uint16_t data)
{
    data_object_t *data_obj = (data_object_t *) object;
    data_object_t *data_out = (data_object_t *) output;

    byte_string_t *new_byte_string;
    char temp_string[BOOL_STRING_SIZE];

    int printed = snprintf(temp_string, BOOL_STRING_SIZE, "%s",
                           (data_obj->payload.bool_value) ? BOOL_STRING_TRUE : BOOL_STRING_FALSE);

    string_cache_status_e cache_err = string_cache_add(temp_string, printed + 1, &new_byte_string);
    if (STRING_CACHE_OK != cache_err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "string_cache_add failed, status %d", cache_err);
        return TYPE_RUNTIME_ERROR;
    }

    memcpy(&data_out->payload.string_value, new_byte_string, sizeof(byte_string_t));
    data_out->data_type = DATATYPE_STRING;
    return TYPE_OK;
}


/* Arithmetic functions */
static type_status_e _arith_int_int(object_t *int_a, object_t *int_b, object_t *result,
                           arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) int_a;
    data_object_t *data_b = (data_object_t *) int_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_INT;

    vm_int_t result_int;

    switch (arith_type)
    {
        case ARITH_ADD:
            result_int = data_a->payload.int_value + data_b->payload.int_value;
            break;

        case ARITH_SUB:
            result_int = data_a->payload.int_value - data_b->payload.int_value;
            break;

        case ARITH_MULT:
            result_int = data_a->payload.int_value * data_b->payload.int_value;
            break;

        case ARITH_DIV:
            result_int = data_a->payload.int_value / data_b->payload.int_value;
            break;

        default:
            result_int = 0;
            break;
    }

    data_result->payload.int_value = result_int;
    return TYPE_OK;
}


static type_status_e _arith_int_float(object_t *int_a, object_t *float_b,
                             object_t *result, arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) int_a;
    data_object_t *data_b = (data_object_t *) float_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_FLOAT;

    vm_float_t result_float;

    switch (arith_type)
    {
        case ARITH_ADD:
            result_float = (vm_float_t) data_a->payload.int_value +
                           data_b->payload.float_value;
            break;

        case ARITH_SUB:
            result_float = (vm_float_t) data_a->payload.int_value -
                           data_b->payload.float_value;
            break;

        case ARITH_MULT:
            result_float = (vm_float_t) data_a->payload.int_value *
                           data_b->payload.float_value;
            break;

        case ARITH_DIV:
            result_float = (vm_float_t) data_a->payload.int_value /
                           data_b->payload.float_value;
            break;

        default:
            result_float = 0.0f;
            break;
    }

    data_result->payload.float_value = result_float;
    return TYPE_OK;
}


static type_status_e _arith_int_string(object_t *int_a, object_t *string_b,
                                       object_t *result, arith_type_e arith_type)
{
    if (ARITH_MULT != arith_type)
    {
        RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                    "Can't perform operation with int and string");
        return TYPE_RUNTIME_ERROR;
    }

    vm_int_t int_value = ((data_object_t *) int_a)->payload.int_value;
    byte_string_t *string_value = &((data_object_t *) string_b)->payload.string_value;
    data_object_t *data_result = (data_object_t *) result;
    byte_string_t *result_string = &data_result->payload.string_value;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_STRING;

    if (TYPE_OK != _multiply_string(int_value, string_value, result_string))
    {
        // _multiply_string only returns success or runtime error
        return TYPE_RUNTIME_ERROR;
    }

    return TYPE_OK;
}


static type_status_e _arith_float_int(object_t *float_a, object_t *int_b,
                             object_t *result, arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) float_a;
    data_object_t *data_b = (data_object_t *) int_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_FLOAT;

    vm_float_t result_float;

    switch (arith_type)
    {
        case ARITH_ADD:
            result_float = data_a->payload.float_value +
                           (vm_float_t) data_b->payload.int_value;
            break;

        case ARITH_SUB:
            result_float = data_a->payload.float_value -
                           (vm_float_t) data_b->payload.int_value;
            break;

        case ARITH_MULT:
            result_float = data_a->payload.float_value *
                           (vm_float_t) data_b->payload.int_value;
            break;

        case ARITH_DIV:
            result_float = data_a->payload.float_value /
                           (vm_float_t) data_b->payload.int_value;
            break;

        default:
            result_float = 0.0f;
            break;
    }

    data_result->payload.float_value = result_float;
    return TYPE_OK;
}


static type_status_e _arith_float_float(object_t *float_a, object_t *float_b,
                               object_t *result, arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) float_a;
    data_object_t *data_b = (data_object_t *) float_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_FLOAT;

    vm_float_t result_float;

    switch (arith_type)
    {
        case ARITH_ADD:
            result_float = data_a->payload.float_value +
                           data_b->payload.float_value;
            break;

        case ARITH_SUB:
            result_float = data_a->payload.float_value -
                           data_b->payload.float_value;
            break;

        case ARITH_MULT:
            result_float = data_a->payload.float_value *
                           data_b->payload.float_value;
            break;

        case ARITH_DIV:
            result_float = data_a->payload.float_value /
                           data_b->payload.float_value;
            break;

        default:
            result_float = 0.0f;
            break;
    }

    data_result->payload.float_value = result_float;
    return TYPE_OK;
}


static type_status_e _arith_string_string(object_t *str_a, object_t *str_b, object_t *result,
                                 arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) str_a;
    data_object_t *data_b = (data_object_t *) str_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_STRING;

    if ((ARITH_DIV == arith_type) ||
        (ARITH_DIV == arith_type) ||
        (ARITH_MULT == arith_type))
    {
            RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                        "Can't perform %s with two strings",
                        (ARITH_DIV == arith_type) ? "Division" : "Subtraction");
            return TYPE_RUNTIME_ERROR;
    }

    byte_string_t *result_string = &data_result->payload.string_value;
    byte_string_t *lhs_string = &data_a->payload.string_value;
    byte_string_t *rhs_string = &data_b->payload.string_value;

    size_t new_string_size = lhs_string->size + rhs_string->size;
    byte_string_t *new_byte_string;
    char *temp_string;

    if ((temp_string = memory_manager_alloc(new_string_size)) == NULL)
    {
        RUNTIME_ERR(RUNTIME_ERROR_MEMORY, "memory_manager_alloc failed");
        return TYPE_RUNTIME_ERROR;
    }

    // Add LHS string to result (skip null byte)
    (void) memcpy(result_string->bytes, lhs_string->bytes,
                  lhs_string->size - 1);

    // Add RHS string to result (skip null byte)
    (void) memcpy(result_string->bytes + (lhs_string->size - 1),
                  rhs_string->bytes, rhs_string->size - 1);


    string_cache_status_e cache_err = string_cache_add(temp_string, new_string_size,
                                                       &new_byte_string);
    if (STRING_CACHE_OK != cache_err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "string_cache_add failed, status %d", cache_err);
        return TYPE_RUNTIME_ERROR;
    }

    memory_manager_free(temp_string);
    memcpy(&data_result->payload.string_value, new_byte_string, sizeof(byte_string_t));
    return TYPE_OK;
}


static type_status_e _arith_string_int(object_t *string_a, object_t *int_b,
                                       object_t *result, arith_type_e arith_type)
{
    if (ARITH_MULT != arith_type)
    {
        RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                    "Can't perform operation with string and int");
        return TYPE_RUNTIME_ERROR;
    }

    vm_int_t int_value = ((data_object_t *) int_b)->payload.int_value;
    byte_string_t *string_value = &((data_object_t *) string_a)->payload.string_value;
    data_object_t *data_result = (data_object_t *) result;
    byte_string_t *result_string = &data_result->payload.string_value;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_STRING;

    if (TYPE_OK != _multiply_string(int_value, string_value, result_string))
    {
        // _multiply_string only returns success or runtime error
        return TYPE_RUNTIME_ERROR;
    }

    return TYPE_OK;
}


static type_status_e _arith_bool_int(object_t *bool_a, object_t *int_b,
                                     object_t *result, arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) bool_a;
    data_object_t *data_b = (data_object_t *) int_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_INT;

    vm_int_t result_int;

    switch (arith_type)
    {
        case ARITH_ADD:
            result_int = (vm_int_t) data_a->payload.bool_value +
                         data_b->payload.int_value;
            break;

        case ARITH_SUB:
            result_int = (vm_int_t) data_a->payload.bool_value -
                         data_b->payload.int_value;
            break;

        case ARITH_MULT:
            result_int = (vm_int_t) data_a->payload.bool_value *
                         data_b->payload.int_value;
            break;

        case ARITH_DIV:
            result_int = (vm_int_t) data_a->payload.bool_value /
                         data_b->payload.int_value;
            break;

        default:
            result_int = 0;
            break;
    }

    data_result->payload.int_value = result_int;
    return TYPE_OK;
}


static type_status_e _arith_bool_float(object_t *bool_a, object_t *float_b,
                                       object_t *result, arith_type_e arith_type)
{
    data_object_t *data_a = (data_object_t *) bool_a;
    data_object_t *data_b = (data_object_t *) float_b;
    data_object_t *data_result = (data_object_t *) result;

    data_result->object.obj_type = OBJTYPE_DATA;
    data_result->data_type = DATATYPE_FLOAT;

    vm_float_t result_float;

    switch (arith_type)
    {
        case ARITH_ADD:
            result_float = (vm_float_t) data_a->payload.bool_value +
                           data_b->payload.float_value;
            break;

        case ARITH_SUB:
            result_float = (vm_float_t) data_a->payload.bool_value -
                           data_b->payload.float_value;
            break;

        case ARITH_MULT:
            result_float = (vm_float_t) data_a->payload.bool_value *
                           data_b->payload.float_value;
            break;

        case ARITH_DIV:
            result_float = (vm_float_t) data_a->payload.bool_value /
                           data_b->payload.float_value;
            break;

        default:
            result_float = 0.0;
            break;
    }

    data_result->payload.float_value = result_float;
    return TYPE_OK;
}


/**
 * @see type_operations_api.h
 */
type_status_e type_arithmetic(object_t *lhs, object_t *rhs, object_t *result,
                              arith_type_e arith_type)
{
    if ((OBJTYPE_DATA != lhs->obj_type) || (OBJTYPE_DATA != rhs->obj_type))
    {
        return TYPE_INVALID_ARITHMETIC;
    }

    data_object_t *rdata = (data_object_t *) rhs;
    data_object_t *ldata = (data_object_t *) lhs;

    type_operations_t *ops = &_type_ops[ldata->data_type];
    arith_func_t arith_func = ops->arith_functions[rdata->data_type];

    if (NULL == arith_func)
    {
        return TYPE_INVALID_ARITHMETIC;
    }

    return arith_func(lhs, rhs, result, arith_type);
}


/**
 * @see type_operations_api.h
 */
type_status_e type_cast_to(object_t *object, object_t *output, data_type_e type,
                           uint16_t data)
{
    if (NUM_DATATYPES <= type)
    {
        return TYPE_ERROR;
    }

    if (OBJTYPE_DATA != object->obj_type)
    {
        return TYPE_INVALID_CAST;
    }

    data_object_t *data_obj = (data_object_t *) object;

    if (data_obj->data_type == type)
    {
        return TYPE_NO_CAST_REQUIRED;
    }

    type_operations_t *ops = &_type_ops[data_obj->data_type];
    cast_func_t cast_func = ops->cast_functions[type];

    if (NULL == cast_func)
    {
        return TYPE_INVALID_CAST;
    }

    output->obj_type = OBJTYPE_DATA;
    return cast_func(object, output, data);
}
