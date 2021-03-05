#include "entity.h"
#include "entity_bucket.h"

entity_bucket *bucket_create() {
	entity_bucket *bucket = (entity_bucket *)malloc(sizeof(entity_bucket));
	
	bucket->arr = (entity_t **)malloc(sizeof(entity_t *) * 2);
	bucket->size = 0;
	bucket->max_size = 2;

	return bucket;
}

void bucket_destroy(entity_bucket *bucket) {
	free(bucket->arr);
	bucket->arr = NULL;
	free(bucket);
	bucket = NULL;
}

void bucket_add(entity_bucket *bucket, entity_t *entity) {
	bucket->arr[bucket->size++] = entity;

	if (bucket->size == bucket->max_size) {
		bucket->max_size <<= 1;
		bucket->arr = (entity_t **)realloc(bucket->arr, sizeof(entity_t *) * bucket->max_size);
	}
}

void bucket_remove(entity_bucket *bucket, entity_t *entity) {
	if (bucket == NULL) {
		printf("error: attempted to remove an entity from a bucket that doesn't exist.\n");
		exit(1);
	}

	int i, j;

	for (i = 0; i < bucket->size; i++) {
		if (bucket->arr[i] == entity) {
			for (j = i + 1; j < bucket->size; j++)
				bucket->arr[j - 1] = bucket->arr[j];
			bucket->size--;

			if (bucket->max_size > 2 && bucket->size < bucket->max_size >> 1) {
				bucket->max_size >>= 1;
				bucket->arr = (entity_t **)realloc(bucket->arr, sizeof(entity_t *) * bucket->max_size);
			}
			break;
		}
	}
}

