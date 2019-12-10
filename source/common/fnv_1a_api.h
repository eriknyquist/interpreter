#ifndef FNV_1A_API_H_
#define FNV_1A_API_H_


#include <stdint.h>
#include <stdlib.h>


uint32_t fnv_1a_32_hash(void *data, size_t size);

uint64_t fnv_1a_64_hash(void *data, size_t size);

#endif /* FNV_1A_API_H_ */
