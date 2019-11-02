#include <stdlib.h>
#include "type_operations_api.h"
#include "opcode_handlers.h"


/* Size allocated for string data in a DATATYPE_STRING object created to
 * hold the string-ified representation of an integer */
#define MAX_STRING_NUM_SIZE (32u)


/* Highest values allowed for number of decimal places when converting
 * a float object to a string object */
#define MAX_FLOAT_PLACES (32)


/* Min/max valid values for base when converting a string object to an int
 * object, as specified by http://man7.org/linux/man-pages/man3/strtol.3.html */
#define MIN_STRING_INT_BASE (2)
#define MAX_STRING_INT_BASE (36)


#define TYPE_OPS_DEF(cast_int, cast_float, cast_string,             \
                     arith_int, arith_float, arith_string)          \
    {.cast_functions={(cast_int), (cast_float), (cast_string)},     \
     .arith_functions={(arith_int), (arith_float), (arith_string)}}


typedef type_status_e (*cast_func_t) (object_t *, object_t *, uint16_t);

typedef type_status_e (*arith_func_t) (object_t *, object_t *, object_t *,
                                       arith_type_e);


typedef struct
{
    cast_func_t cast_functions[NUM_DATATYPES];
    arith_func_t arith_functions[NUM_DATATYPES];
} type_operations_t;


/* Casting functions */
static type_status_e _int_to_float(object_t *, object_t *, uint16_t);     /* Cast int to float */
static type_status_e _int_to_string(object_t *, object_t *, uint16_t);    /* Cast int to string */
static type_status_e _float_to_int(object_t *, object_t *, uint16_t);     /* Cast float to int */
static type_status_e _float_to_string(object_t *, object_t *, uint16_t);  /* Cast float to string */
static type_status_e _string_to_int(object_t *, object_t *, uint16_t);    /* Cast string to int */
static type_status_e _string_to_float(object_t *, object_t *, uint16_t);  /* Cast string to int */


/* Arithmetic when LHS is an int */
static type_status_e _arith_int_int(object_t *, object_t *, object_t *, arith_type_e);
static type_status_e _arith_int_float(object_t *, object_t *, object_t *, arith_type_e);


/* Arithmetic when LHS is a float */
static type_status_e _arith_float_float(object_t *, object_t *, object_t *, arith_type_e);
static type_status_e _arith_float_int(object_t *, object_t *, object_t *, arith_type_e);


/* Arithmetic when LHS is a string */
static type_status_e _arith_string_string(object_t *, object_t *, object_t *, arith_type_e);


static type_operations_t _type_ops[NUM_DATATYPES] =
{
    TYPE_OPS_DEF(NULL, _int_to_float, _int_to_string,
                 _arith_int_int, _arith_int_float, NULL),     // DATATYPE_INT

    TYPE_OPS_DEF(_float_to_int, NULL, _float_to_string,
                 _arith_float_int, _arith_float_float, NULL), // DATATYPE_FLOAT

    TYPE_OPS_DEF(_string_to_int, _string_to_float, NULL,
                 NULL, NULL, _arith_string_string)            // DATATYPE_STRING
};


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
    byte_string_status_e err;

    err = byte_string_create(&data_out->payload.string_value);
    if (BYTE_STRING_OK != err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "byte_string_create failed, status %d", err);
        return TYPE_RUNTIME_ERROR;
    }

    err = byte_string_snprintf(&data_out->payload.string_value,
                               MAX_STRING_NUM_SIZE, "%d", int_value);
    if (BYTE_STRING_OK != err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "byte_string_snprintf failed, status %d", err);
        return TYPE_RUNTIME_ERROR;
    }

    data_out->data_type = DATATYPE_STRING;
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

    byte_string_status_e err;

    if (MAX_FLOAT_PLACES < places)
    {
        RUNTIME_ERR(RUNTIME_ERROR_CAST,
                    "decimal places must be between 0-%d\n", MAX_FLOAT_PLACES);
        return TYPE_RUNTIME_ERROR;
    }

    err = byte_string_create(&data_out->payload.string_value);
    if (BYTE_STRING_OK != err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "byte_string_create failed, status %d\n", err);
        return TYPE_RUNTIME_ERROR;
    }

    // Build format string with requested number of places
    char fmt_string[16];
    if (snprintf(fmt_string, sizeof(fmt_string), "%%.%df", places) >= sizeof(fmt_string))
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL, "snprintf output truncated\n");
        return TYPE_RUNTIME_ERROR;
    }

    err = byte_string_snprintf(&data_out->payload.string_value,
                                MAX_STRING_NUM_SIZE + places, fmt_string,
                                data_obj->payload.float_value);
    if (BYTE_STRING_OK != err)
    {
        RUNTIME_ERR(RUNTIME_ERROR_INTERNAL,
                    "byte_string_snprintf failed, status %d", err);
        return TYPE_RUNTIME_ERROR;
    }

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

    switch (arith_type)
    {
        case ARITH_DIV:
        case ARITH_SUB:
            RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                        "Can't perform %s with two strings",
                        (ARITH_DIV == arith_type) ? "Division" : "Subtraction");
            return TYPE_RUNTIME_ERROR;

            break;

        default:
            // Nothing to do
            break;
    }

    byte_string_t *result_string = &data_result->payload.string_value;
    byte_string_t *lhs_string = &data_a->payload.string_value;
    byte_string_t *rhs_string = &data_b->payload.string_value;
    byte_string_status_e err;

    if (BYTE_STRING_OK != (err = byte_string_create(result_string)))
    {
        RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                    "byte_string_create failed, status %d", err);
        return TYPE_RUNTIME_ERROR;
    }

    switch (arith_type)
    {
        case ARITH_ADD:
            // Add LHS string to result
            if (BYTE_STRING_OK != (err = byte_string_add_bytes(result_string,
                                                               lhs_string->used_bytes,
                                                               lhs_string->bytes)))
            {
                RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                            "byte_string_add_bytes failed, status %d", err);
                return TYPE_RUNTIME_ERROR;
            }

            // Add RHS string to result
            if (BYTE_STRING_OK != (err = byte_string_add_bytes(result_string,
                                                               rhs_string->used_bytes,
                                                               rhs_string->bytes)))
            {
                RUNTIME_ERR(RUNTIME_ERROR_ARITHMETIC,
                            "byte_string_add_bytes failed, status %d", err);
            }

            // Ensure string is NULL terminated
            result_string->bytes[result_string->used_bytes] = '\0';
            break;

        case ARITH_MULT:
            break;

        default:
            // Nothing to do
            break;
    }

    return TYPE_OK;
}


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
        return TYPE_OK;
    }

    type_operations_t *ops = &_type_ops[data_obj->data_type];
    cast_func_t cast_func = ops->cast_functions[type];

    if (NULL == cast_func)
    {
        return TYPE_INVALID_CAST;
    }

    return cast_func(object, output, data);
}
