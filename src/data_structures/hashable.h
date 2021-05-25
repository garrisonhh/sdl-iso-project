#ifndef HASHABLE_H
#define HASHABLE_H

typedef unsigned hash_t;

enum hashable_e {
	HASH_UNSIGNED,
	HASH_STRING,
	HASH_V3I,
};
typedef enum hashable_e hashable_e;

struct hashmap_funcs_t {
	int (*cmp)(const void *, const void *);
	hash_t (*hash)(const void *);
	const void *(*copy)(const void *);
};
typedef struct hashmap_funcs_t hashmap_funcs_t;

hashmap_funcs_t hashmap_funcs(hashable_e);

#endif
