#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "hashmap.h"
#include "hashable.h"

#include <stdio.h> // TODO remove
#include "../vector.h" // TODO REMOVE

#define HASHKEY(hmap, key) (hmap->funcs.hash(key) % hmap->max_size)

#define BUCKET_MATCH(hmap, bucket, key, hash) (bucket.hash == hash && !hmap->funcs.cmp(bucket.key, key))
#define FIND_BUCKET_IDX(hmap, key, hash, idx) {\
	idx = hash;\
	while (hmap->buckets[idx].key != NULL && !BUCKET_MATCH(hmap, hmap->buckets[idx], key, hash))\
		idx = (idx + 1) % hmap->max_size;\
}

// structs are typedef'd in header
struct hashbucket_t {
	hash_t hash;
	const void *key, *value;
};

struct hashmap_iter_t {
	hashmap_t *hmap;
	int index;
};

void hashmap_clear(hashmap_t *);
void hashmap_rehash(hashmap_t *hmap, size_t new_size);

hashmap_t *hashmap_create(size_t initial_size, hashable_e hash_type) {
	hashmap_t *hmap = malloc(sizeof(hashmap_t));

	hmap->max_size = initial_size;
	hmap->size = 0;
	hmap->min_size = initial_size;

	hmap->buckets = malloc(sizeof(hashbucket_t) * hmap->max_size);
	hashmap_clear(hmap);

	hmap->funcs = hashmap_funcs(hash_type);

	return hmap;
}

void hashmap_destroy(hashmap_t *hmap, bool destroy_values) {
	for (size_t i = 0; i < hmap->max_size; ++i) {
		if (hmap->buckets[i].key != NULL) {
			free((void *)hmap->buckets[i].key);

			if (destroy_values && hmap->buckets[i].value != NULL)
				free((void *)hmap->buckets[i].value);
		}
	}

	free(hmap->buckets);
	free(hmap);
}

void hashmap_clear(hashmap_t *hmap) {
	for (size_t i = 0; i < hmap->max_size; ++i) {
		hmap->buckets[i].hash = 0;
		hmap->buckets[i].key = NULL;
		hmap->buckets[i].value = NULL;
	}
}

void hashmap_set_lower(hashmap_t *hmap, const void *key, const void *value, bool copy) {
	if (hmap->size > (hmap->max_size >> 1))
		hashmap_rehash(hmap, hmap->max_size << 1);

	hash_t idx, hash = HASHKEY(hmap, key);

	FIND_BUCKET_IDX(hmap, key, hash, idx);

	hmap->buckets[idx].value = value;

	if (hmap->buckets[idx].key == NULL) {
		hmap->buckets[idx].hash = hash;
		hmap->buckets[idx].key = (copy ? hmap->funcs.copy(key) : key);

		++hmap->size;
	}
}

void hashmap_rehash(hashmap_t *hmap, size_t new_size) {
	hashbucket_t *old_buckets = hmap->buckets;
	size_t old_size = hmap->max_size;

	hmap->size = 0;
	hmap->max_size = new_size;
	hmap->buckets = malloc(sizeof(hashbucket_t) * hmap->max_size);

	hashmap_clear(hmap);

	for (size_t i = 0; i < old_size; ++i)
		if (old_buckets[i].key != NULL)
			hashmap_set_lower(hmap, old_buckets[i].key, old_buckets[i].value, false);

	free(old_buckets);
}

void *hashmap_get(hashmap_t *hmap, const void *key) {
	hash_t idx, hash = HASHKEY(hmap, key);

	FIND_BUCKET_IDX(hmap, key, hash, idx);

	return (hmap->buckets[idx].key == NULL ? NULL : (void *)hmap->buckets[idx].value);
}

void hashmap_set(hashmap_t *hmap, const void *key, const void *value) {
	hashmap_set_lower(hmap, key, value, true);
}

void *hashmap_remove(hashmap_t *hmap, const void *key) {
	hash_t idx, hash = HASHKEY(hmap, key);

	FIND_BUCKET_IDX(hmap, key, hash, idx);

	if (hmap->buckets[idx].key == NULL) {
		return NULL;
	} else {
		void *value = (void *)hmap->buckets[idx].value;
		free((void *)hmap->buckets[idx].key);

		hash_t last_slot = idx;
		hashbucket_t *bucket;

		while (hmap->buckets[(idx = (idx + 1) % hmap->max_size)].key != NULL) {
			bucket = &hmap->buckets[idx];

			if ((bucket->hash < idx && (last_slot < idx && bucket->hash <= last_slot))
			 || (bucket->hash > idx && ((last_slot > idx && bucket->hash <= last_slot) || last_slot < idx))) {
				hmap->buckets[last_slot] = hmap->buckets[idx];
				last_slot = idx;
			}
		}

		hmap->buckets[last_slot].hash = 0;
		hmap->buckets[last_slot].key = NULL;
		hmap->buckets[last_slot].value = NULL;

		if (--hmap->size < (hmap->max_size >> 2)) {
			hashmap_rehash(hmap, hmap->max_size >> 1);
		}

		return value;
	}
}

hashmap_iter_t *hashmap_iter_create(hashmap_t *hmap) {
	hashmap_iter_t *iter = malloc(sizeof(hashmap_iter_t));

	iter->hmap = hmap;
	iter->index = 0;

	return iter;
}

void hashmap_iter_reset(hashmap_iter_t *iter) {
	iter->index = 0;
}

void *hashmap_iter_next(hashmap_iter_t *iter) {
	do {
		if (iter->hmap->buckets[iter->index].key != NULL)
			return (void *)iter->hmap->buckets[iter->index++].value;
	} while (++iter->index < iter->hmap->max_size);

	return NULL;
}
