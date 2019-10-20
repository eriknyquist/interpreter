#ifndef DATA_TYPES_H_
#define DATA_TYPES_H_


#include "ulist_api.h"


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
        long int int_value;  // DATATYPE_INT
        double float_value;  // DATATYPE_FLOAT
        char *string;        // DATATYPE_STRING
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
