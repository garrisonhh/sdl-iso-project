#ifndef HASH_H
#define HASH_H

#include <stdlib.h>

typedef size_t hash_t;

typedef struct hash_bucket {
	void *key;
	size_t size_key;
	void *value;
	struct hash_bucket *overflow;
} hash_bucket;

typedef struct hash_table {
	hash_bucket **buckets;
	hash_t (*hash_func)(const void *, size_t);
	size_t size, entries;
} hash_table;

hash_table *hash_table_create(size_t initial_size, hash_t (*hash_func)(const void *, size_t));
void hash_table_destroy(hash_table *);
void hash_table_deep_destroy(hash_table *);

void *hash_get(hash_table *, void *, size_t);
hash_t hash_set(hash_table *, void *, size_t, void *value);
void hash_remove(hash_table *, void *, size_t);

#endif

