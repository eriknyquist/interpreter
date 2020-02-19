#include <string.h>

#include "object_helpers_api.h"
#include "memory_manager_api.h"
#include "string_cache_api.h"


#define NEW_OBJECT(size, obj)                                                 \
{                                                                             \
    obj = memory_manager_alloc(size);                                         \
    if (NULL == obj)                                                          \
    {                                                                         \
        return NULL;                                                          \
    }                                                                         \
                                                                              \
    obj->refcount = 0u;                                                       \
}                                                                             \


/**
 * @see object_helpers_api.h
 */
object_t *new_int_object(vm_int_t value)
{
    object_t *new_obj;
    NEW_OBJECT(sizeof(data_object_t), new_obj);

    data_object_t *data_obj = (data_object_t *) new_obj;

    data_obj->object.obj_type = OBJTYPE_DATA;
    data_obj->data_type = DATATYPE_INT;
    data_obj->payload.int_value = value;
    return new_obj;
}


/**
 * @see object_helpers_api.h
 */
object_t *new_float_object(vm_float_t value)
{
    object_t *new_obj;
    NEW_OBJECT(sizeof(data_object_t), new_obj);

    data_object_t *data_obj = (data_object_t *) new_obj;

    data_obj->object.obj_type = OBJTYPE_DATA;
    data_obj->data_type = DATATYPE_FLOAT;
    data_obj->payload.float_value = value;
    return new_obj;
}


/**
 * @see object_helpers_api.h
 */
object_t *new_bool_object(vm_bool_t value)
{
    object_t *new_obj;
    NEW_OBJECT(sizeof(data_object_t), new_obj);

    data_object_t *data_obj = (data_object_t *) new_obj;

    data_obj->object.obj_type = OBJTYPE_DATA;
    data_obj->data_type = DATATYPE_BOOL;
    data_obj->payload.bool_value = value;
    return new_obj;
}


/**
 * @see object_helpers_api.h
 */
object_t *new_string_object(char *string, size_t len)
{
    object_t *new_obj;
    NEW_OBJECT(sizeof(data_object_t), new_obj);

    data_object_t *data_obj = (data_object_t *) new_obj;

    data_obj->object.obj_type = OBJTYPE_DATA;
    data_obj->data_type = DATATYPE_STRING;

    byte_string_t *new_byte_string;
    string_cache_status_e cache_err = string_cache_add(string, len,
                                                       &new_byte_string);
    if (STRING_CACHE_ALREADY_CACHED < cache_err)
    {
        return NULL;
    }

    memcpy(&data_obj->payload.string_value, new_byte_string,
           sizeof(byte_string_t));

    return new_obj;
}
