#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap.h"
#include "../utils.h"

struct hashbucket_t {
	void *key;
	size_t size_key;
	void *value;
	struct hashbucket_t *overflow;
};

hashmap_t *hashmap_create(size_t initial_size, bool rehashes, hash_t (*hash_func)(const void *, size_t)) {
	hashmap_t *hmap = (hashmap_t *)malloc(sizeof(hashmap_t));

	hmap->max_size = initial_size;
	hmap->size = 0;
	hmap->buckets = (hashbucket_t **)malloc(sizeof(hashbucket_t *) * hmap->max_size);
	hmap->hash_func = hash_func;
	hmap->rehashes = rehashes;

	for (size_t i = 0; i < hmap->max_size; i++)
		hmap->buckets[i] = NULL;

	return hmap;
}

hashbucket_t *hashbucket_destroy(hashbucket_t *bucket, bool destroy_value) {
	hashbucket_t *overflow = bucket->overflow;

	if (destroy_value && bucket->value != NULL)
		free(bucket->value);

	free(bucket->key);
	free(bucket);

	return overflow;
}

void hashmap_destroy(hashmap_t *hmap, bool destroy_values) {
	hashbucket_t *trav;

	// I like this code right here. C can be so much more concise than you expect
	for (size_t i = 0; i < hmap->max_size; i++)
		if ((trav = hmap->buckets[i]) != NULL)
			while ((trav = hashbucket_destroy(trav, destroy_values)) != NULL)
				;

	free(hmap->buckets);
	free(hmap);
}

hash_t hash_key(hashmap_t *hmap, void *key, size_t size_key) {
	return hmap->hash_func(key, size_key) % hmap->max_size;
}

void hashmap_rehash(hashmap_t *hmap) {
	size_t old_size = hmap->max_size, i;
	hashbucket_t **old_buckets = hmap->buckets;
	hashbucket_t *trav;

	hmap->max_size <<= 1;
	hmap->buckets = (hashbucket_t **)malloc(sizeof(hashbucket_t *) * hmap->max_size);
	hmap->size = 0;

	for (i = 0; i < hmap->max_size; i++)
		hmap->buckets[i] = NULL;

	for (i = 0; i < old_size; i++) {
		if ((trav = old_buckets[i]) != NULL) {
			while (trav != NULL) {
				hashmap_set(hmap, trav->key, trav->size_key, trav->value);
				trav = trav->overflow;
			}

			hashbucket_destroy(old_buckets[i], false);
		}
	}

	free(old_buckets);
}

hashbucket_t *hashmap_get_pair(hashmap_t *hmap, void *key, size_t size_key) {
	hash_t hash = hash_key(hmap, key, size_key);
	hashbucket_t *trav = hmap->buckets[hash];

	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key)))
		trav = trav->overflow;

	return trav;
}

void *hashmap_get(hashmap_t *hmap, void *key, size_t size_key) {
	hashbucket_t *bucket = hashmap_get_pair(hmap, key, size_key);

	return (bucket != NULL ? bucket->value : NULL);
}

void hashmap_remove(hashmap_t *hmap, void *key, size_t size_key, bool destroy_value) {
	hash_t hash;
	hashbucket_t *trav, *last;

	hash = hash_key(hmap, key, size_key);
	last = NULL;
	trav = hmap->buckets[hash];

	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key))) {
		last = trav;
		trav = trav->overflow;
	}

	if (last != NULL)
		last->overflow = hashbucket_destroy(trav, destroy_value);
}

hash_t hashmap_set(hashmap_t *hmap, void *key, size_t size_key, void *value) {
	hash_t hash;
	hashbucket_t *bucket, *trav;

	hash = hash_key(hmap, key, size_key);
	trav = hmap->buckets[hash];

	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key)))
		trav = trav->overflow;

	if (trav != NULL) { // found matching bucket, modify value
		printf("modifying value\n");
		trav->value = value;
	} else { // no matching bucket, create new bucket
		bucket = (hashbucket_t *)malloc(sizeof(hashbucket_t));

		bucket->key = malloc(size_key);
		memcpy(bucket->key, key, size_key);

		bucket->size_key = size_key;
		bucket->value = value;
		bucket->overflow = NULL;

		if (hmap->buckets[hash] == NULL)
			hmap->buckets[hash] = bucket;
		else {
			trav = hmap->buckets[hash];

			while (trav->overflow != NULL)
				trav = trav->overflow;

			trav->overflow = bucket;
		}

		if (hmap->rehashes && ++hmap->size == hmap->max_size)
			hashmap_rehash(hmap);
	}

	return hash;
}

// useful for iteration
void **hashmap_values(hashmap_t *hmap) {
	void **values;
	hashbucket_t *trav;
	size_t i, cur_values;

	values = (void **)calloc(hmap->size, sizeof(void *));
	cur_values = 0;

	for (i = 0; i < hmap->max_size; i++) {
		trav = hmap->buckets[i];

		while (trav != NULL) {
			values[cur_values++] = trav->value;
			trav = trav->overflow;
		}
	}

	return values;
}

