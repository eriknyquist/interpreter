#ifndef RUNTIME_COMMON_H_
#define RUNTIME_COMMON_H_


#include "data_types.h"
#include "byte_string_api.h"
#include "ulist_api.h"
#include "runtime_error_api.h"


#define __RUNTIME_ERR(file, line, error_code, fmt, ...)                       \
    do {                                                                      \
        fprintf(stderr, "[%s:%d] "fmt"\n", file, line, ##__VA_ARGS__);        \
        runtime_error_set(error_code);                                        \
    }                                                                         \
    while(0)

#define RUNTIME_ERR(error_code, fmt, ...) __RUNTIME_ERR(__FILE__, __LINE__, error_code, fmt, ##__VA_ARGS__)


#define CHECK_ULIST_ERR_RT(func)                                              \
    do {                                                                      \
        ulist_status_e __err_code = func;                                     \
                                                                              \
        if (ULIST_OK != __err_code)                                           \
        {                                                                     \
            runtime_error_e __rt_err;                                         \
            if (ULIST_ERROR_MEM == __err_code)                                \
            {                                                                 \
                __rt_err = RUNTIME_ERROR_MEMORY;                              \
            }                                                                 \
            else                                                              \
            {                                                                 \
                __rt_err = RUNTIME_ERROR_INTERNAL;                            \
            }                                                                 \
                                                                              \
            RUNTIME_ERR(__rt_err,                                             \
                        "ulist_t operation failed, status %d", __err_code);   \
            return NULL;                                                      \
        }                                                                     \
    }                                                                         \
    while(0)


#define CHECK_STRING_CACHE_ERR_RT(func)                                          \
    do {                                                                         \
        string_cache_status_e __err_code = func;                                 \
                                                                                 \
        if (STRING_CACHE_OK != __err_code)                                       \
        {                                                                        \
            runtime_error_e __rt_err;                                            \
            if (STRING_CACHE_MEMORY_ERROR == __err_code)                         \
            {                                                                    \
                __rt_err = RUNTIME_ERROR_MEMORY;                                 \
            }                                                                    \
            else                                                                 \
            {                                                                    \
                __rt_err = RUNTIME_ERROR_INTERNAL;                               \
            }                                                                    \
                                                                                 \
            RUNTIME_ERR(__rt_err,                                                \
                        "string cache operation failed, status %d",              \
                        __err_code);                                             \
            return NULL;                                                         \
        }                                                                        \
    }                                                                            \
    while(0)


#define CHECK_BYTESTR_ERR_RT(func)                                            \
    do {                                                                      \
        byte_string_status_e __err_code = func;                               \
                                                                              \
        if (BYTE_STRING_OK != __err_code)                                     \
        {                                                                     \
            runtime_error_e __rt_err;                                         \
            if (BYTE_STRING_MEMORY_ERROR == __err_code)                       \
            {                                                                 \
                __rt_err = RUNTIME_ERROR_MEMORY;                              \
            }                                                                 \
            else                                                              \
            {                                                                 \
                __rt_err = RUNTIME_ERROR_INTERNAL;                            \
            }                                                                 \
                                                                              \
            RUNTIME_ERR(__rt_err,                                             \
                        "byte_string_t operation failed, status %d",          \
                        __err_code);                                          \
            return NULL;                                                      \
        }                                                                     \
    }                                                                         \
    while(0)


#define FREE_DATA_OBJECT(obj_ptr)                                                          \
    do {                                                                                   \
        data_object_t *__data_ptr = (data_object_t *) obj_ptr;                             \
        if (DATATYPE_STRING == __data_ptr->data_type)                                      \
        {                                                                                  \
            CHECK_BYTESTR_ERR_RT(byte_string_destroy(&__data_ptr->payload.string_value));  \
        }                                                                                  \
    }                                                                                      \
    while (0)


/* Structure for data representing a VM instance */
typedef struct
{
    runtime_error_e runtime_error;
    callstack_t callstack;
    ulist_t constants;
} vm_instance_t;


#endif /* RUNTIME_COMMON_H_ */
