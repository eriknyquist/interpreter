/*
 * FNV-1a hash implementation, stolen from:
 * http://www.isthe.com/chongo/src/fnv/hash_32a.c
 * http://www.isthe.com/chongo/src/fnv/hash_64a.c
 */


#include "fnv_1a_api.h"


// 32 bit magic FNV-1a prime
#define FNV_32A_PRIME ((uint32_t) 0x01000193u)

// 32 bit magic initial value
#define FNV_32A_INIT ((uint32_t) 0x811c9dc5u)

// 64 bit magic FNV-1a prime
#define FNV_64A_PRIME ((uint64_t) 0x100000001b3u)

// 64 bit magic initial value
#define FNV_64A_INIT ((uint64_t) 0xcbf29ce484222325u)


uint32_t fnv_1a_32_hash(void *data, size_t size)
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


uint64_t fnv_1a_64_hash(void *data, size_t size)
{
    uint8_t *u8_data = data;
	uint64_t hval = FNV_64A_INIT;

    /*
     * FNV-1 hash each octet of the buffer
     */
    for (int i = 0; i < size; i++)
    {
		hval ^= u8_data[i];
		hval *= FNV_64A_PRIME;
    }

    /* return our new hash value */
    return hval;
}
