#include "hashmap.h"
#include "../vector.h"

hash_t hash_unsigned(const void *u_num, size_t size_key) {
	hash_t hash = *(unsigned *)u_num;

	for (int i = 0; i < 3; i++)
		hash = hash * 37 + hash;

	return hash;
}

hash_t hash_string(const void *string, size_t size_key) {
	hash_t hash = 0;
	char *str = (char *)string;

	for (int i = 0; i < size_key; i++)
		hash = (hash << 2) | ((hash_t)*str);

	return hash;
}

hash_t hash_v3i(const void *v3i_ptr, size_t size_key) {
	v3i v = *(v3i *)v3i_ptr;
	unsigned hashable = v.x + v.y + v.z; 

	return hash_unsigned(&hashable, sizeof hashable);
}
