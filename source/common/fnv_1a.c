/*
 * FNV-1a hash implementation, stolen from:
 * http://www.isthe.com/chongo/src/fnv/hash_32a.c
 */


#include "fnv_1a_api.h"


// 32 bit magic FNV-1a prime
#define FNV_32A_PRIME (0x01000193u)

// 32 bit magic initial value
#define FNV_32A_INIT (0x811c9dc5u)


uint32_t fnv_1a_hash(void *data, size_t size)
{   
    uint8_t * u8_data = data;
    uint32_t hash = FNV_32A_INIT;

    for (int i = 0; i < size; i++)
    {
        hash ^= u8_data[i];
        hash *= FNV_32A_PRIME;
    }

    return hash;
}
