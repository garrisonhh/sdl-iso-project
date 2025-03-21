#define GHH_LIB_INTERNAL
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ghh/heap.h>
#include <ghh/memcheck.h>

struct ghh_heap {
	void **items;
	size_t max_size, size;
	int (*compare)(const void *, const void *);
};

heap_t *heap_create(int initial_depth, int (*compare)(const void *, const void *)) {
	heap_t *heap = malloc(sizeof(heap_t));

	heap->max_size = (1 << initial_depth) - 1;
	heap->size = 0;
	heap->items = malloc(heap->max_size * sizeof(void *));
	heap->compare = compare;

	for (size_t i = 0; i < heap->max_size; i++)
		heap->items[i] = NULL;

	return heap;
}

void heap_destroy(heap_t *heap, bool destroy_values) {
	if (destroy_values)
		for (size_t i = 0; i < heap->max_size; i++)
			free(heap->items[i]);

	free(heap->items);
	free(heap);
}

size_t heap_size(heap_t *heap) {
	return heap->size;
}

static inline void heap_swap_indices(heap_t *heap, size_t a, size_t b) {
	void *swap = heap->items[a];
	heap->items[a] = heap->items[b];
	heap->items[b] = swap;
}

void heap_sift_down(heap_t *heap) {
	size_t index, child_a, child_b;
	bool swap_a, swap_b;

	index = 0;
	child_a = 1;
	child_b = 2;

	do {
		swap_a = child_a < heap->size && heap->items[child_a] != NULL
			  && heap->compare(heap->items[index], heap->items[child_a]) > 0;
		swap_b = child_b < heap->size && heap->items[child_b] != NULL
			  && heap->compare(heap->items[index], heap->items[child_b]) > 0;

		if (swap_a && swap_b) {
			if (heap->compare(heap->items[child_b], heap->items[child_a]) > 0)
				swap_b = false;
			else
				swap_a = false;
		}

		if (swap_a) {
			heap_swap_indices(heap, child_a, index);
			index = child_a;
		} else if (swap_b) {
			heap_swap_indices(heap, child_b, index);
			index = child_b;
		}

		child_a = index << 1;
		child_b = child_a + 1;
	} while (swap_a || swap_b);
}

void heap_insert(heap_t *heap, void *item) {
	size_t index, other;

	heap->items[(index = heap->size)] = item;

	// sift up
	while (index > 0 && heap->compare(heap->items[(other = index >> 1)], heap->items[index]) > 0) {
		heap_swap_indices(heap, index, other);
		index = other;
	}

	if (++heap->size == heap->max_size) {
		heap->max_size = (heap->max_size << 1) | 1;
		heap->items = realloc(heap->items, sizeof(void *) * heap->max_size);
	}
}

void *heap_pop(heap_t *heap) {
	if (heap->size == 0) {
		printf("attempted to pop an empty heap.\n");
		exit(1);
	}

	void *top = heap->items[0];
	heap->items[0] = heap->items[--heap->size];
	heap->items[heap->size] = NULL;
	heap_sift_down(heap);

	return top;
}

void *heap_peek(heap_t *heap) {
	if (heap->size == 0) {
		printf("attempted to peek an empty heap.\n");
		exit(1);
	}

	return heap->items[0];
}

void *heap_replace(heap_t *heap, void *item) {
	if (heap->size == 0) {
		printf("attempted to replace in an empty heap.\n");
		exit(1);
	}

	void *top = heap->items[0];
	heap->items[0] = item;
	heap_sift_down(heap);

	return top;
}
