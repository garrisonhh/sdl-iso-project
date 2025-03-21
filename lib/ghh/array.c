#define GHH_LIB_INTERNAL
#include <stdlib.h>
#include <stdbool.h>
#include <ghh/array.h>
#include <ghh/memcheck.h>

struct ghh_array {
	void **items;
	size_t size, min_size, alloc_size;
};

array_t *array_create(size_t initial_size) {
	array_t *array = malloc(sizeof(*array));

	array->size = 0;
	array->min_size = (initial_size < 4 ? 4 : initial_size);
	array->alloc_size = array->min_size;
	array->items = malloc(array->alloc_size * sizeof(void *));

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

void *array_get(array_t *array, size_t index) {
	return array->items[index];
}

void *array_set(array_t *array, size_t index, void *item) {
	void *old = array->items[index];

	array->items[index] = item;

	return old;
}

void *array_raw(array_t *array) {
	return array->items;
}
void array_push(array_t *array, void *item) {
	if (array->size == array->alloc_size) {
		array->alloc_size <<= 1;
		array->items = realloc(array->items, sizeof(void *) * array->alloc_size);
	}

	array->items[array->size++] = item;
}

void *array_pop(array_t *array) {
	if (!array->size)
		return NULL;

	void *value = array->items[--array->size];

	if (array->size < (array->alloc_size >> 1)) {
		array->alloc_size >>= 1;
		array->items = realloc(array->items, sizeof(void *) * array->alloc_size);
	}

	return value;
}

void *array_peek(array_t *array) {
	return array->items[array->size - 1];
}

void *array_del(array_t *array, size_t index) {
	void *item = array->items[index];

	for (size_t i = index; i < array->size - 1; ++i)
		array->items[i] = array->items[i + 1];

	--array->size;

	if (array->alloc_size > array->min_size && array->size < array->alloc_size >> 1) {
		array->alloc_size >>= 1;
		array->items = realloc(array->items, sizeof(void *) * array->alloc_size);
	}

	return item;
}

void array_clear(array_t *array, bool destroy_values) {
	if (destroy_values)
		for (size_t i = 0; i < array->size; i++)
			free(array->items[i]);

	array->size = 0;
	array->alloc_size = array->min_size;

	free(array->items);
	array->items = malloc(sizeof(void *) * array->alloc_size);
}

void array_qsort(array_t *array, int (*compare)(const void *, const void *)) {
	qsort(array->items, array->size, sizeof(void *), compare);
}
