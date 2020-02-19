#ifndef OBJECT_HELPERS_API_H
#define OBJECT_HELPERS_API_H

#include "data_types.h"


/**
 * Allocate a new int object, intialize it with the given value and return
 * a pointer to the new object
 *
 * @param  value    Initial value
 *
 * @return   Pointer to allocated object, NULL if allocation was unsuccessful
 */
object_t *new_int_object(vm_int_t value);


/**
 * Allocate a new float object, intialize it with the given value and return
 * a pointer to the new object
 *
 * @param  value    Initial value
 *
 * @return   Pointer to allocated object, NULL if allocation was unsuccessful
 */
object_t *new_float_object(vm_float_t value);


/**
 * Allocate a new bool object, intialize it with the given value and return
 * a pointer to the new object
 *
 * @param  value    Initial value
 *
 * @return   Pointer to allocated object, NULL if allocation was unsuccessful
 */
object_t *new_bool_object(vm_bool_t value);


/**
 * Allocate a new string object, intialize it with the given value and return
 * a pointer to the new object
 *
 * @param  string    Pointer to initial bytes for string value
 * @param  len       Number of bytes to copy from string data pointer
 *
 * @return   Pointer to allocated object, NULL if allocation was unsuccessful
 */
object_t *new_string_object(char *string, size_t len);


#endif
