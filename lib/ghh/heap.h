#ifndef GHH_HEAP_H
#define GHH_HEAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ghh_heap heap_t;

heap_t *heap_create(int initial_depth, int (*compare)(const void *, const void *));
void heap_destroy(heap_t *, bool destroy_values);

size_t heap_size(heap_t *);

void heap_insert(heap_t *, void *item);
void *heap_pop(heap_t *);
void *heap_peek(heap_t *);
void *heap_replace(heap_t *, void *item);

#endif
