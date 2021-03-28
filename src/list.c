#include <stdlib.h>
#include "list.h"

list_t *list_create() {
	list_t *list = (list_t *)malloc(sizeof(list_t));
	
	list->size = 0;
	list->max_size = 4;
	list->items = (void **)malloc(sizeof(void *) * list->max_size);

	return list;
}

void list_destroy(list_t *list) {
	free(list->items);
	free(list);
}

// also destroys items in list
void list_deep_destroy(list_t *list) {
	for (size_t i = 0; i < list->size; i++)
		free(list->items[i]);
	list_destroy(list);
}

void list_add(list_t *list, void *item) {
	list->items[list->size++] = item;

	if (list->size == list->max_size) {
		list->max_size <<= 1;
		list->items = realloc(list->items, sizeof(void *) * list->max_size);
	}
}

void list_remove(list_t *list, void *item) {
	int i, j;

	for (i = 0; i < list->size; i++) {
		if (list->items[i] == item) {
			for (j = i + 1; j < list->size; j++)
				list->items[j - 1] = list->items[j];
			list->size--;

			if (list->max_size > 4 && list->size < list->max_size >> 1) {
				list->max_size >>= 1;
				list->items = (void **)realloc(list->items, sizeof(void *) * list->max_size);
			}
			break;
		}
	}
}

void list_qsort(list_t *list, int (*compare)(const void *, const void *)) {
	qsort(list->items, list->size, sizeof(void *), compare);
}
