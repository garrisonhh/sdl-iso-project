#ifndef GHH_HASH_COMMON_H
#define GHH_HASH_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <ghh/hash.h>
#define GHH_MEMCHECK_OVERRIDES
#include <ghh/memcheck.h>
#undef GHH_MEMCHECK_OVERRIDES

static inline hash_t hash_key(int key_size, const void *key) {
	return key_size < 0 ? hash(key, strlen(key)) : hash(key, key_size);
}

static inline const void *copy_key(int key_size, const void *key) {
	void *copied;

	if (key_size < 0) {
		size_t num_bytes = 1;
		char *str = (char *)key;

		while (*str++)
			++num_bytes;

		num_bytes *= sizeof(*str);

		copied = ghh_override_malloc(num_bytes);
		memcpy(copied, key, num_bytes);
	} else {
		copied = ghh_override_malloc(key_size);
		memcpy(copied, key, key_size);
	}

	return copied;
}

static inline bool key_equals(int key_size, const void *key, const void *other) {
	size_t i = 0;
	const uint8_t *kbytes = key, *obytes = other;

	if (key_size < 0) {
		do {
			if (kbytes[i] != obytes[i])
				return false;
		} while (kbytes[i++]);
	} else {
		for (; i < key_size; ++i)
			if (kbytes[i] != obytes[i])
				return false;
	}

	return true;
}

#endif
