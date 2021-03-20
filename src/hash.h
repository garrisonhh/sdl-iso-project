#ifndef HASH_H
#define HASH_H

#include <stdlib.h>

typedef size_t hash_t;

typedef struct hash_bucket {
	const char * key;
	size_t len_key;
	void *value;
	struct hash_bucket *overflow;
} hash_bucket;

typedef struct hash_table {
	hash_bucket **buckets;
	size_t size, entries;
} hash_table;

hash_table *hash_table_create(size_t);
void hash_table_destroy(hash_table *);
void hash_table_deep_destroy(hash_table *);

void *hash_get(hash_table *, const char *);
hash_t hash_set(hash_table *, const char *, void *);
void hash_remove(hash_table *, const char *);

#endif

