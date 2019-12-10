#ifndef FNV_1A_API_H_
#define FNV_1A_API_H_


#include <stdint.h>
#include <stdlib.h>


/**
 * Calculate the FNV-1a 32-bit hash of the provided data. Taken from reference
 * code at http://www.isthe.com/chongo/src/fnv/hash_32a.c
 *
 * @param   data    Pointer to data to hash
 * @param   size    Size of data in bytes
 *
 * @return          The hash of the provided data
 */
uint32_t fnv_1a_32_hash(void *data, size_t size);


/**
 * Calculate the FNV-1a 64-bit hash of the provided data. Taken from reference
 * code at http://www.isthe.com/chongo/src/fnv/hash_64a.c
 *
 * @param   data    Pointer to data to hash
 * @param   size    Size of data in bytes
 *
 * @return          The hash of the provided data
 */
uint64_t fnv_1a_64_hash(void *data, size_t size);


#endif /* FNV_1A_API_H_ */
