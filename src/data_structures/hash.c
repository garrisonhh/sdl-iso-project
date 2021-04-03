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

void hash_bucket_destroy(hash_bucket *bucket) {
	if (bucket->overflow != NULL)
		hash_bucket_destroy(bucket->overflow);

	free(bucket->key);
	free(bucket);
}

void hash_bucket_deep_destroy(hash_bucket *bucket) {
	if (bucket->value != NULL)
		free(bucket->value);

	hash_bucket_destroy(bucket);
}

void hash_table_destroy(hash_table *table) {
	for (size_t i = 0; i < table->size; i++)
		if (table->buckets[i] != NULL)
			hash_bucket_destroy(table->buckets[i]);

	free(table->buckets);
	free(table);
}

void hash_table_deep_destroy(hash_table *table) {
	for (size_t i = 0; i < table->size; i++)
		if (table->buckets[i] != NULL)
			hash_bucket_deep_destroy(table->buckets[i]);

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

			hash_bucket_destroy(old_buckets[i]);
		}
	}

	free(old_buckets);
}

hash_bucket *hash_get_pair(hash_table *table, void *key, size_t size_key) {
	hash_t hash = hash_key(table, key, size_key);
	hash_bucket *trav = table->buckets[hash];

	//printf("new traversal \"%s\" %lu:\n", (char *)key, size_key);
	while (trav != NULL && (size_key != trav->size_key || memcmp(key, trav->key, trav->size_key))) {
		//printf("traversing \"%s\" %lu\n", (char *)trav->key, trav->size_key);
		trav = trav->overflow;
	}

	return trav;
}

void *hash_get(hash_table *table, void *key, size_t size_key) {
	hash_bucket *bucket = hash_get_pair(table, key, size_key);

	return (bucket != NULL ? bucket->value : NULL);
}

void hash_remove(hash_table *table, void *key, size_t size_key) {
	printf("UNIMPLEMENTED!!!\n");
	exit(1);
}

// if unused bucket pointers are not NULL, this will segfault
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

