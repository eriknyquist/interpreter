#include "runtime_error_api.h"


static runtime_error_e _runtime_error = RUNTIME_ERROR_NONE;


void runtime_error_set(runtime_error_e error)
{
    _runtime_error = error;
}


runtime_error_e runtime_error_get(void)
{
    return _runtime_error;
}
