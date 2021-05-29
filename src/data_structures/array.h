#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stdlib.h>
#include <stdbool.h>

struct array_t {
	void **items;
	size_t max_size, size, min_size;
};
typedef struct array_t array_t;

array_t *array_create(size_t initial_size);
void array_destroy(array_t *, bool destroy_values);

void array_push(array_t *, void *item);
void *array_pop(array_t *);
void array_remove(array_t *, void *item);
void array_clear(array_t *, bool destroy_values);
void array_qsort(array_t *, int (*compare)(const void *, const void *));

#endif
