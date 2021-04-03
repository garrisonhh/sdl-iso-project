#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "../utils.h"

hash_table *hash_table_create(size_t initial_size, hash_t (*hash_func)(const void *, size_t)) {
	hash_table *table = (hash_table *)malloc(sizeof(hash_table));

	table->size = initial_size;
	table->entries = 0;
	table->buckets = (hash_bucket **)malloc(sizeof(hash_bucket *) * table->size);
	table->hash_func = hash_func;

	for (size_t i = 0; i < table->size; i++)
		table->buckets[i] = NULL;

	return table;
}

hash_bucket *hash_bucket_destroy(hash_bucket *bucket, bool destroy_value) {
	hash_bucket *overflow = bucket->overflow;

	if (destroy_value && bucket->value != NULL)
		free(bucket->value);

	free(bucket->key);
	free(bucket);

	return overflow;
}

void hash_table_destroy(hash_table *table, bool destroy_values) {
	hash_bucket *trav;

	// I like this code right here. C can be so much more concise than you expect
	for (size_t i = 0; i < table->size; i++)
		if ((trav = table->buckets[i]) != NULL)
			while ((trav = hash_bucket_destroy(trav, destroy_values)) != NULL)
				;

	free(table->buckets);
	free(table);
}

hash_t hash_key(hash_table *table, void *key, size_t size_key) {
	return table->hash_func(key, size_key) % table->size;
}

void hash_rehash(hash_table *table) {
	size_t old_size = table->size, i;
	hash_bucket **old_buckets = table->buckets;
	hash_bucket *trav;

	table->size <<= 1;
	table->buckets = (hash_bucket **)malloc(sizeof(hash_bucket *) * table->size);
	table->entries = 0;

	for (i = 0; i < table->size; i++)
		table->buckets[i] = NULL;

	for (i = 0; i < old_size; i++) {
		if ((trav = old_buckets[i]) != NULL) {
			while (trav != NULL) {
				hash_set(table, trav->key, trav->size_key, trav->value);
				trav = trav->overflow;
			}

			hash_bucket_destroy(old_buckets[i], false);
		}
	}

	free(old_buckets);
}

hash_bucket *hash_get_pair(hash_table *table, void *key, size_t size_key) {
	hash_t hash = hash_key(table, key, size_key);
	hash_bucket *trav = table->buckets[hash];

	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key)))
		trav = trav->overflow;

	return trav;
}

void *hash_get(hash_table *table, void *key, size_t size_key) {
	hash_bucket *bucket = hash_get_pair(table, key, size_key);

	return (bucket != NULL ? bucket->value : NULL);
}

void hash_remove(hash_table *table, void *key, size_t size_key, bool destroy_value) {
	hash_t hash;
	hash_bucket *trav, *last;

	hash = hash_key(table, key, size_key);
	last = NULL;
	trav = table->buckets[hash];

	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key))) {
		last = trav;
		trav = trav->overflow;
	}

	if (last != NULL)
		last->overflow = hash_bucket_destroy(trav, destroy_value);
}

hash_t hash_set(hash_table *table, void *key, size_t size_key, void *value) {
	hash_bucket *bucket, *trav;
	hash_t hash;

	hash = hash_key(table, key, size_key);
	trav = table->buckets[hash];

	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key)))
		trav = trav->overflow;

	if (trav != NULL) { // found matching bucket, modify value
		trav->value = value;
	} else { // no matching bucket, create new bucket
		bucket = (hash_bucket *)malloc(sizeof(hash_bucket));

		bucket->key = malloc(size_key);
		memcpy(bucket->key, key, size_key);

		bucket->size_key = size_key;
		bucket->value = value;
		bucket->overflow = NULL;

		if (table->buckets[hash] == NULL)
			table->buckets[hash] = bucket;
		else {
			trav = table->buckets[hash];

			while (trav->overflow != NULL)
				trav = trav->overflow;

			trav->overflow = bucket;
		}

		if (++table->entries == table->size)
			hash_rehash(table);
	}

	return hash;
}

