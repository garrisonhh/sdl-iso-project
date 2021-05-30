#include <stdlib.h>
#include <string.h>
#include "hashable.h"
#include "../lib/vector.h"

hash_t hash_unsigned(const void *u_num) {
	hash_t hash = *(unsigned *)u_num;

	for (int i = 0; i < 4; i++)
		hash = hash + (hash ^ (hash * 33));

	return hash;
}

int cmp_unsigned(const void *a, const void *b) {
	return *(unsigned *)a - *(unsigned *)b;
}

const void *copy_unsigned(const void *u) {
	unsigned *ucpy = malloc(sizeof(unsigned));
	*ucpy = *(unsigned *)u;

	return ucpy;
}

hash_t hash_string(const void *string) {
	hash_t hash = 0;
	char *str = (char *)string;

	while (*str++)
		hash = (hash << 2) | ((hash_t)*str);

	return hash;
}

int cmp_string(const void *a, const void *b) {
	return strcmp(a, b);
}

const void *copy_string(const void *string) {
	char *str = malloc(sizeof(char) * (strlen(string) + 1));

	strcpy(str, string);

	return str;
}

hash_t hash_v3i(const void *v3i_ptr) {
	v3i v = *(v3i *)v3i_ptr;
	unsigned hashable = v.x + v.y + v.z; 

	return hash_unsigned(&hashable);
}

int cmp_v3i(const void *a, const void *b) {
	return v3i_compare(*(v3i *)a, *(v3i *)b);
}

const void *copy_v3i(const void *v3i_ptr) {
	v3i *vcpy = malloc(sizeof(v3i));

	*vcpy = *(v3i *)v3i_ptr;

	return vcpy;
}

hashmap_funcs_t hashmap_funcs(hashable_e hashable) {
	switch (hashable) {
	case HASH_UNSIGNED:
		return (hashmap_funcs_t){
			.cmp = cmp_unsigned,
			.hash = hash_unsigned,
			.copy = copy_unsigned
			};
		case HASH_STRING:
			return (hashmap_funcs_t){
				.cmp = cmp_string,
				.hash = hash_string,
				.copy = copy_string
			};
		case HASH_V3I:
			return (hashmap_funcs_t){
				.cmp = cmp_v3i,
				.hash = hash_v3i,
				.copy = copy_v3i
			};
		default:
			exit(1);
	}
}
