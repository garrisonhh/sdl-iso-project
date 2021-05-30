#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

#include "hashable.h"

typedef struct hashbucket_t hashbucket_t;
typedef struct hashmap_t hashmap_t;
typedef struct hashmap_iter_t hashmap_iter_t;

struct hashmap_t {
	hashbucket_t *buckets;
	size_t max_size, size, min_size;
	hashmap_funcs_t funcs;
};

// hashmap
hashmap_t *hashmap_create(size_t initial_size, hashable_e hash_type);
void hashmap_destroy(hashmap_t *, bool destroy_values);

void *hashmap_get(hashmap_t *, const void *key);
void hashmap_set(hashmap_t *, const void *key, const void *value);
void *hashmap_remove(hashmap_t *, const void *key);

void hashmap_print(hashmap_t *, size_t, size_t);

// hashmap iterator
hashmap_iter_t *hashmap_iter_create(hashmap_t *);

void hashmap_iter_reset(hashmap_iter_t *);
void *hashmap_iter_next(hashmap_iter_t *); // returns NULL when done

#endif
