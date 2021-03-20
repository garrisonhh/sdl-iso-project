#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "utils.h"

hash_t hash_key(hash_table *table, char *key, size_t len_key) {
	hash_t sum;
	int max_index = MIN(3, len_key);

	for (int i = 0; i < max_index; i++)
		sum += key[i];

	return sum % table->size;
}

hash_table *hash_table_create(size_t initial_size) {
	hash_table *new_table = (hash_table *)malloc(sizeof(hash_table));

	new_table->size = initial_size;
	new_table->entries = 0;
	new_table->buckets = (hash_bucket **)malloc(sizeof(hash_bucket *) * new_table->size);

	for (int i = 0; i < new_table->size; i++)
		new_table->buckets[i] = NULL;

	return new_table;
}

void hash_bucket_destroy(hash_bucket *bucket) {
	if (bucket->overflow != NULL)
		hash_bucket_destroy(bucket->overflow);
	free(bucket);
}

void hash_bucket_deep_destroy(hash_bucket *bucket) {
	if (bucket->value != NULL)
		free(bucket->value);
	hash_bucket_destroy(bucket);
}

void hash_table_destroy(hash_table *table) {
	for (size_t i = 0; i < table->size; i++) {
		if (table->buckets[i] != NULL)
			hash_bucket_destroy(table->buckets[i]);
	}

	free(table);
}

void hash_table_deep_destroy(hash_table *table) {
	for (size_t i = 0; i < table->size; i++) {
		if (table->buckets[i] != NULL)
			hash_bucket_deep_destroy(table->buckets[i]);
	}

	free(table);
}

void hash_rehash(hash_table *table) {
	size_t old_size = table->size;
	hash_bucket **old_buckets = table->buckets;
	hash_bucket *trav;

	table->size <<= 1;
	table->buckets = (hash_bucket **)malloc(sizeof(hash_bucket *) * table->size);

	for (size_t i = 0; i < old_size; i++) {
		trav = old_buckets[i];

		while (trav != NULL) {
			hash_set(table, trav->key, trav->value);
			trav = trav->overflow;
		}

		hash_bucket_destroy(old_buckets[i]);
	}
}

hash_bucket *hash_get_pair(hash_table *table, char *key) {
	size_t len_key = strlen(key);
	hash_t hash = hash_key(table, key, len_key);
	hash_bucket *trav;

	trav = table->buckets[hash];

	while (trav != NULL) {
		if (!strcmp(key, trav->key))
			return trav;
		trav = trav->overflow;
	}

	return NULL;
}

void *hash_get(hash_table *table, char *key) {
	hash_bucket *bucket;

	return (bucket = hash_get_pair(table, key)) != NULL ? bucket->value : NULL;
}

void hash_remove(hash_table *table, char *key) {
	unsigned int hash = hash_key(table, key, strlen(key));

	if (table->buckets[hash] != NULL) {
		hash_bucket_destroy(table->buckets[hash]);
		table->buckets[hash] = NULL;
	}
}

hash_t hash_set(hash_table *table, char *key, void *value) {
	hash_bucket *bucket;
	hash_t hash;
	size_t len_key;

	len_key = strlen(key);
	hash = hash_key(table, key, len_key);

	if ((bucket = hash_get_pair(table, key)) != NULL) {
		bucket->value = value;
	} else {
		bucket = (hash_bucket *)malloc(sizeof(hash_bucket));

		bucket->key = key;
		bucket->len_key = len_key;
		bucket->value = value;
		bucket->overflow = NULL;
		
		if (table->buckets[hash] == NULL)
			table->buckets[hash] = bucket;
		else {
			hash_bucket *trav = table->buckets[hash];

			while (trav->overflow != NULL)
				trav = trav->overflow;

			trav->overflow = bucket;
		}

		if (++table->entries == table->size)
			hash_rehash(table);
	}

	return hash;
}

