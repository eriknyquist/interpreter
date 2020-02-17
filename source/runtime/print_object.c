#include <stdio.h>
#include "print_object_api.h"


static void print_data_obj (data_object_t *data_obj)
{
    switch (data_obj->data_type)
    {
        case DATATYPE_INT:
            printf("%d\n", data_obj->payload.int_value);
            break;

        case DATATYPE_FLOAT:
            printf("%.4f\n", data_obj->payload.float_value);
            break;

        case DATATYPE_BOOL:
            printf("%s\n", data_obj->payload.bool_value ? "True" : "False");
            break;

        case DATATYPE_STRING:
            printf("%s\n", data_obj->payload.string_value.bytes);
            break;

        default:
            printf("Unable to print data type %d\n", data_obj->data_type);
            break;
    }
}


void print_object(object_t *object)
{
    if (NULL == object)
    {
        printf("NULL\n");
    }

    switch (object->obj_type)
    {
        case OBJTYPE_DATA:
        {
            data_object_t *data_obj =  (data_object_t *) object;
            print_data_obj(data_obj);
            break;
        }

        case OBJTYPE_FUNCTION:
            // TODO
            break;

        case OBJTYPE_CLASS:
            // TODO
            break;

        case OBJTYPE_INSTANCE:
            // TODO
            break;

        default:
            printf("Unable to print object type %d\n", object->obj_type);
            break;
    }
}
