#define GHH_LIB_INTERNAL
#include <stdlib.h>
#include <string.h>
#include <ghh/hashmap.h>
#include <ghh/utils.h>
#define GHH_MEMCHECK_OVERRIDES
#include <ghh/memcheck.h>
#undef GHH_MEMCHECK_OVERRIDES

#include "hash_common.h"

#define MIN_HASHMAP_SIZE 8

typedef struct hashbucket {
	bool filled;
	int desired_idx;
	hash_t hash;
	void *key, *value;
} hashbucket_t;

struct ghh_hashmap {
	hashbucket_t *buckets;
	size_t size, min_size, alloc_size;

	int key_size;
	bool copy_keys;
};

struct ghh_hmapiter {
	hashmap_t *hmap;
	int idx;
};

hashmap_t *hashmap_create(size_t initial_size, int key_size, bool copy_keys) {
	hashmap_t *hmap = ghh_override_malloc(sizeof(*hmap));

	hmap->key_size = key_size;
	hmap->copy_keys = copy_keys;

	hmap->size = 0;
	hmap->min_size = MAX(initial_size, MIN_HASHMAP_SIZE);
	hmap->alloc_size = hmap->min_size;
	hmap->buckets = ghh_override_calloc(hmap->alloc_size, sizeof(*hmap->buckets));

	return hmap;
}

void hashmap_destroy(hashmap_t *hmap, bool destroy_values) {
	if (destroy_values)
		for (size_t i = 0; i < hmap->alloc_size; ++i)
			if (hmap->buckets[i].filled)
				free(hmap->buckets[i].value);

	if (hmap->copy_keys)
		for (size_t i = 0; i < hmap->alloc_size; ++i)
			if (hmap->buckets[i].key != NULL)
				ghh_override_free(hmap->buckets[i].key);

	ghh_override_free(hmap->buckets);
	ghh_override_free(hmap);
}

size_t hashmap_size(hashmap_t *hmap) {
	return hmap->size;
}

// returns -1 if no bucket found
static inline int get_bucket_index(hashmap_t *hmap, const void *key, hash_t hash) {
	int index = hash % hmap->alloc_size;

	while (hmap->buckets[index].filled) {
		if (hmap->buckets[index].hash == hash
		 && key_equals(hmap->key_size, key, hmap->buckets[index].key)) {
			break;
		}

		index = (index + 1) % hmap->alloc_size;
	}

	return index;
}

// returns old value, null if none found
static inline void *hashmap_set_lower(hashmap_t *hmap, void *key, void *value, hash_t hash) {
	int index;
	void *old_value = NULL;

	index = get_bucket_index(hmap, key, hash);

	if (hmap->buckets[index].filled) {
		if (hmap->copy_keys)
			ghh_override_free(hmap->buckets[index].key);

		old_value = hmap->buckets[index].value;
	} else {
		hmap->buckets[index].filled = true;
		hmap->buckets[index].hash = hash;

		++hmap->size;
	}

	hmap->buckets[index].key = key;
	hmap->buckets[index].value = value;

	// this should only be needed for rehash, TODO some fix?
	hmap->buckets[index].desired_idx = hmap->buckets[index].hash % hmap->alloc_size;

	return old_value;
}

static inline void rehash(hashmap_t *hmap, size_t new_size) {
	if (new_size < hmap->min_size)
		return;

	hashbucket_t *old_buckets;
	size_t old_size;

	old_buckets = hmap->buckets;
	old_size = hmap->alloc_size;

	hmap->size = 0;
	hmap->alloc_size = new_size;
	hmap->buckets = ghh_override_calloc(hmap->alloc_size, sizeof(*hmap->buckets));

	for (size_t i = 0; i < old_size; ++i) {
		if (old_buckets[i].filled) {
			hashmap_set_lower(
				hmap,
				old_buckets[i].key,
				old_buckets[i].value,
				old_buckets[i].hash
			);
		}
	}

	ghh_override_free(old_buckets);
}

void *hashmap_get(hashmap_t *hmap, const void *key) {
	int index = get_bucket_index(hmap, key, hash_key(hmap->key_size, key));

	return hmap->buckets[index].filled ? hmap->buckets[index].value : NULL;
}

bool hashmap_may_get(hashmap_t *hmap, const void *key, void **out_value) {
	int index = get_bucket_index(hmap, key, hash_key(hmap->key_size, key));

	if (hmap->buckets[index].filled) {
		if (out_value != NULL)
			*out_value = hmap->buckets[index].value;

		return true;
	}

	return false;
}

bool hashmap_has(hashmap_t *hmap, const void *key) {
	return hmap->buckets[get_bucket_index(hmap, key, hash_key(hmap->key_size, key))].filled;
}

void *hashmap_set(hashmap_t *hmap, const void *key, const void *value) {
	if (hmap->size >= (hmap->alloc_size >> 1))
		rehash(hmap, hmap->alloc_size << 1);

	if (hmap->copy_keys)
		key = copy_key(hmap->key_size, key);

	return hashmap_set_lower(hmap, (void *)key, (void *)value, hash_key(hmap->key_size, key));
}

void *hashmap_remove(hashmap_t *hmap, const void *key) {
	int index;
	void *value = NULL;

	index = get_bucket_index(hmap, key, hash_key(hmap->key_size, key));

	if (hmap->buckets[index].filled) {
		// filter up slots connected to this one
		int last_index = index, desired;
		bool filter;

		value = hmap->buckets[index].value;

		if (hmap->copy_keys)
			ghh_override_free(hmap->buckets[index].key);

		while (1) {
			// increment with wrap
			index = (index + 1) % hmap->alloc_size;

			// stop if an empty slot is reached
			if (!hmap->buckets[index].filled)
				break;

			// flag whether to filter up
			filter = false;
			desired = hmap->buckets[index].desired_idx;

			if (desired < index) { // no bucket wrap
				if (last_index < index && desired <= last_index)
					filter = true;
			} else if (desired > index) { // bucket wrap
				if (last_index < index || (/*last_index > index &&*/ desired <= last_index))
					filter = true;
			}

			// filter up
			if (filter) {
				/*printf(
					"filtering up %d <- %d (wants %d)\n",
					last_index, index, hmap->buckets[index].desired_idx
				);*/
				hmap->buckets[last_index] = hmap->buckets[index];
				last_index = index;
			}
		}

		hmap->buckets[last_index].filled = false;

		if (--hmap->size < (hmap->alloc_size >> 2))
			rehash(hmap, hmap->alloc_size >> 1);
	}

	return value;
}

hmapiter_t *hmapiter_create(hashmap_t *hmap) {
	hmapiter_t *iter = ghh_override_malloc(sizeof(*iter));

	iter->hmap = hmap;
	iter->idx = -1;

	return iter;
}

void hmapiter_reset(hmapiter_t *iter) {
	iter->idx = -1;
}

// return whether iteration is not done
bool hmapiter_next(hmapiter_t *iter, void **out_key, void **out_value) {
	do {
		if (++iter->idx >= iter->hmap->alloc_size) {
			iter->idx = -1;

			if (out_key != NULL)
				*out_key = NULL;
			if (out_value != NULL)
				*out_value = NULL;

			return false;
		}
	} while (!iter->hmap->buckets[iter->idx].filled);

	if (out_key != NULL)
		*out_key = iter->hmap->buckets[iter->idx].key;
	if (out_value != NULL)
		*out_value = iter->hmap->buckets[iter->idx].value;

	return true;
}
