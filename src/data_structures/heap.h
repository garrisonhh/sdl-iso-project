#ifndef HEAP_H
#define HEAP_H

#include <stdlib.h>

struct heap_t {
	void **items;
	size_t size, entries;
	int (*compare)(const void *, const void *);
};
typedef struct heap_t heap_t;

heap_t *heap_create(int (*compare)(const void *, const void *));
void heap_destroy(heap_t *, bool destroy_values);

void heap_insert(heap_t *, void *item);
void *heap_pop(heap_t *);
void *heap_peek(heap_t *);
void *heap_replace(heap_t *, void *item);

#endif
