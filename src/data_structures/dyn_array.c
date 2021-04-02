#include <stdlib.h>
#include "dyn_array.h"

dyn_array_t *dyn_array_create() {
	dyn_array_t *dyn_array = (dyn_array_t *)malloc(sizeof(dyn_array_t));
	
	dyn_array->size = 0;
	dyn_array->max_size = 4;
	dyn_array->items = (void **)malloc(sizeof(void *) * dyn_array->max_size);

	return dyn_array;
}

void dyn_array_destroy(dyn_array_t *dyn_array) {
	free(dyn_array->items);
	free(dyn_array);
}

// also destroys items in dyn_array
void dyn_array_deep_destroy(dyn_array_t *dyn_array) {
	for (size_t i = 0; i < dyn_array->size; i++)
		free(dyn_array->items[i]);
	dyn_array_destroy(dyn_array);
}

void dyn_array_add(dyn_array_t *dyn_array, void *item) {
	dyn_array->items[dyn_array->size++] = item;

	if (dyn_array->size == dyn_array->max_size) {
		dyn_array->max_size <<= 1;
		dyn_array->items = realloc(dyn_array->items, sizeof(void *) * dyn_array->max_size);
	}
}

void dyn_array_remove(dyn_array_t *dyn_array, void *item) {
	int i, j;

	for (i = 0; i < dyn_array->size; i++) {
		if (dyn_array->items[i] == item) {
			for (j = i + 1; j < dyn_array->size; j++)
				dyn_array->items[j - 1] = dyn_array->items[j];
			dyn_array->size--;

			if (dyn_array->max_size > 4 && dyn_array->size < dyn_array->max_size >> 1) {
				dyn_array->max_size >>= 1;
				dyn_array->items = (void **)realloc(dyn_array->items, sizeof(void *) * dyn_array->max_size);
			}
			break;
		}
	}
}

void dyn_array_qsort(dyn_array_t *dyn_array, int (*compare)(const void *, const void *)) {
	qsort(dyn_array->items, dyn_array->size, sizeof(void *), compare);
}
