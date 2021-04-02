#include "hash.h"

hash_t hash_string(const void *string, size_t size_key) {
	hash_t hash = 0;
	char *str = (char *)string;

	do {
		hash = (hash << 2) | ((hash_t)*str);
	} while (*++str);

	return hash;
}
