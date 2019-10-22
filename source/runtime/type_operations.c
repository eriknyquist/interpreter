#include "type_operations_api.h"


#define TYPE_OPS_DEF(cast_int, cast_float, cast_string,             \
                     arith_int, arith_float, arith_string)          \
    {.cast_functions={(cast_int), (cast_float), (cast_string)},     \
     .arith_functions={(arith_int), (arith_float), (arith_string)}}


typedef void (*cast_func_t) (object_t *);
typedef void (*arith_func_t) (object_t *, object_t *, object_t *, arith_type_e);


typedef struct
{
    cast_func_t cast_functions[NUM_DATATYPES];
    arith_func_t arith_functions[NUM_DATATYPES];
} type_operations_t;


static void _int_to_float(object_t *);
static void _float_to_int(object_t *);

static void _arith_int_int(object_t *, object_t *, object_t *, arith_type_e);
static void _arith_int_float(object_t *, object_t *, object_t *, arith_type_e);
static void _arith_float_float(object_t *, object_t *, object_t *, arith_type_e);
static void _arith_float_int(object_t *, object_t *, object_t *, arith_type_e);


static type_operations_t _type_ops[NUM_DATATYPES] =
{
    TYPE_OPS_DEF(NULL, _int_to_float, NULL,
                 _arith_int_int, _arith_int_float, NULL),     // DATATYPE_INT

    TYPE_OPS_DEF(_float_to_int, NULL, NULL,
                 _arith_float_int, _arith_float_float, NULL), // DATATYPE_FLOAT

    TYPE_OPS_DEF(NULL, NULL, NULL,
                 NULL, NULL, NULL)                            // DATATYPE_STRING
};


static void _int_to_float(object_t *object)
{
    data_object_t *data_obj = (data_object_t *) object;

    data_obj->data_type = DATATYPE_FLOAT;
    data_obj->payload.float_value = (vm_float_t) data_obj->payload.int_value;
}


static void _float_to_int(object_t *object)
{
    data_object_t *data_obj = (data_object_t *) object;

    data_obj->data_type = DATATYPE_INT;
    data_obj->payload.int_value = (vm_int_t) data_obj->payload.float_value;
}


static void _arith_int_int(object_t *int_a, object_t *int_b, object_t *result,
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
}


static void _arith_int_float(object_t *int_a, object_t *float_b,
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
}


static void _arith_float_int(object_t *float_a, object_t *int_b,
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
}

static void _arith_float_float(object_t *float_a, object_t *float_b,
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

    arith_func(lhs, rhs, result, arith_type);
    return TYPE_OK;
}


type_status_e type_cast_to(object_t *object, data_type_e type)
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

    cast_func(object);
    return TYPE_OK;
}
