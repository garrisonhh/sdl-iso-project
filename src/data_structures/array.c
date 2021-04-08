#include <stdlib.h>
#include <stdbool.h>
#include "array.h"

struct array_t {
	void **items;
	size_t max_size, size;
};

array_t *array_create() {
	array_t *array = (array_t *)malloc(sizeof(array_t));
	
	array->size = 0;
	array->max_size = 4;
	array->items = (void **)malloc(sizeof(void *) * array->max_size);

	return array;
}

void array_destroy(array_t *array, bool destroy_values) {
	if (destroy_values)
		for (size_t i = 0; i < array->size; i++)
			free(array->items[i]);

	free(array->items);
	free(array);
}

size_t array_size(array_t *array) {
	return array->size;
}

void array_add(array_t *array, void *item) {
	array->items[array->size++] = item;

	if (array->size == array->max_size) {
		array->max_size <<= 1;
		array->items = realloc(array->items, sizeof(void *) * array->max_size);
	}
}

void array_remove(array_t *array, void *item) {
	int i, j;

	for (i = 0; i < array->size; i++) {
		if (array->items[i] == item) {
			for (j = i + 1; j < array->size; j++)
				array->items[j - 1] = array->items[j];
			array->size--;

			if (array->max_size > 4 && array->size < array->max_size >> 1) {
				array->max_size >>= 1;
				array->items = (void **)realloc(array->items, sizeof(void *) * array->max_size);
			}
			break;
		}
	}
}

void array_qsort(array_t *array, int (*compare)(const void *, const void *)) {
	qsort(array->items, array->size, sizeof(void *), compare);
}
