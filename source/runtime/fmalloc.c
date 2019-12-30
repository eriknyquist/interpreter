/*
 * Fixed-size block allocator for small allocations ( < 512 bytes). Heavily
 * influenced by Python's obmalloc.c, although far simpler.
 */


/* Defined for clarity/readability, but this value cannot be changed in
 * isolation, since we are making the assumption that size classes are
 * seperated by 8 byte intervals throughout this file (e.g. 8, 16, 24,
 * and so on). */
#define ALIGNMENT_BYTES (8)


// Map index to block size (multiply by 8 with bit shifts)
#define ITOBS(i) ((i + 1) << 3)


// Map block size to index
#define BSTOI(s) ((s >> 3) - 1)


