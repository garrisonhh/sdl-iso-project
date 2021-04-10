#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef unsigned hash_t;
typedef struct hashbucket_t hashbucket_t;

struct hashmap_t {
	hashbucket_t **buckets;
	size_t max_size, size;
	hash_t (*hash_func)(const void *, size_t);
	bool rehashes; // when false, hashmap_set() will never rehash
};
typedef struct hashmap_t hashmap_t;

hashmap_t *hashmap_create(size_t initial_size, bool rehashes, hash_t (*hash_func)(const void *, size_t));
void hashmap_destroy(hashmap_t *, bool destroy_values);

void *hashmap_get(hashmap_t *, void *key, size_t size_key);
hash_t hashmap_set(hashmap_t *, void *key, size_t size_key, void *value);
void hashmap_remove(hashmap_t *, void *key, size_t size_key, bool destroy_value);
void **hashmap_values(hashmap_t *);

#endif

