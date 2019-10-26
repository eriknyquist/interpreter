#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_

#include <stdint.h>
#include "ulist_api.h"
#include "byte_string_api.h"


/* Mapping between C types and virtual machine types */
typedef int32_t vm_int_t;
typedef double vm_float_t;


/**
 * Enumerations of all possible object types
 */
typedef enum
{
    OBJTYPE_DATA,
    OBJTYPE_FUNCTION,
    NUM_OBJTYPES
} object_type_e;


/**
 * Enumerations of all possible data types
 */
typedef enum
{
    DATATYPE_INT,
    DATATYPE_FLOAT,
    DATATYPE_STRING,
    NUM_DATATYPES
} data_type_e;


/* Structure representing all objects */
typedef struct
{
    object_type_e obj_type;
} object_t;


/**
 * Structure representing a data object
 */
typedef struct
{
    object_t object;
    data_type_e data_type;
    union {
        vm_int_t int_value;               // DATATYPE_INT
        vm_float_t float_value;           // DATATYPE_FLOAT
        byte_string_t string_value;       // DATATYPE_STRING
    } payload;
} data_object_t;


/**
 * Structure representing a single frame within the call stack
 */
typedef struct
{
    ulist_t data;
} callstack_frame_t;


/**
 * Structure representing a call stack for the running program
 */
typedef struct
{
    ulist_t frames;
    callstack_frame_t *current_frame;
} callstack_t;


#endif /* DATA_TYPES_H_ */
