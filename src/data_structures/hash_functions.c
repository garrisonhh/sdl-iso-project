#include "hashmap.h"

hash_t hash_string(const void *string, size_t size_key) {
	hash_t hash = 0;
	char *str = (char *)string;

	for (int i = 0; i < size_key; i++)
		hash = (hash << 2) | ((hash_t)*str);

	return hash;
}
