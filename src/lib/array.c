#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "array.h"

array_t *array_create(size_t initial_size) {
	array_t *array = malloc(sizeof(array_t));
	
	array->size = 0;
	array->min_size = (initial_size < 4 ? 4 : initial_size);
	array->max_size = array->min_size;
	array->items = malloc(sizeof(void *) * array->max_size);

	return array;
}

void array_destroy(array_t *array, bool destroy_values) {
	if (destroy_values)
		for (size_t i = 0; i < array->size; i++)
			free(array->items[i]);

	free(array->items);
	free(array);
}

void array_push(array_t *array, void *item) {
	if (array->size == array->max_size) {
		array->max_size <<= 1;
		array->items = realloc(array->items, sizeof(void *) * array->max_size);
	}

	array->items[array->size++] = item;
}

void *array_pop(array_t *array) {
	void *value = array->items[--array->size];

	if (array->size < (array->max_size >> 1)) {
		array->max_size >>= 1;
		array->items = realloc(array->items, sizeof(void *) * array->max_size);
	}

	return value;
}

void *array_del(array_t *array, int index) {
	void *item = array->items[index];

	for (int i = index + 1; i < array->size; ++i)
		array->items[i - 1] = array->items[i];

	--array->size;

	if (array->max_size > array->min_size && array->size < array->max_size >> 1) {
		array->max_size >>= 1;
		array->items = realloc(array->items, sizeof(void *) * array->max_size);
	}

	return item;
}

void array_remove(array_t *array, void *item) {
	int i, j;

	for (i = 0; i < array->size; i++) {
		if (array->items[i] == item) {
			for (j = i + 1; j < array->size; j++)
				array->items[j - 1] = array->items[j];

			--array->size;

			if (array->max_size > array->min_size && array->size < array->max_size >> 1) {
				array->max_size >>= 1;
				array->items = realloc(array->items, sizeof(void *) * array->max_size);
			}
			break;
		}
	}
}

void array_clear(array_t *array, bool destroy_values) {
	if (destroy_values)
		for (size_t i = 0; i < array->size; i++)
			free(array->items[i]);

	array->size = 0;
	array->max_size = array->min_size;

	free(array->items);
	array->items = malloc(sizeof(void *) * array->max_size);
}


void array_qsort(array_t *array, int (*compare)(const void *, const void *)) {
	qsort(array->items, array->size, sizeof(void *), compare);
}
