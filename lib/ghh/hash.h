#ifndef GHH_HASH_H
#define GHH_HASH_H

#include <stddef.h>
#include <stdint.h>

#if INTPTR_MAX == INT64_MAX

// 64 bit
typedef uint64_t hash_t;
#define FNV_PRIME 0x00000100000001b3
#define FNV_BASIS 0xcbf29ce484222325

#else

// 32 bit
typedef uint32_t hash_t;
#define FNV_PRIME 0x01000193a
#define FNV_BASIS 0x811c9dc5

#endif

// fnv-1a hash function (http://isthe.com/chongo/tech/comp/fnv/)
static inline hash_t hash(const void *data, const size_t size_data) {
	hash_t hash = FNV_BASIS;
	const uint8_t *bytes = data;

	for (int i = 0; i < size_data; ++i) {
		hash ^= bytes[i];
		hash *= FNV_PRIME;
	}

	return hash;
}

#endif
