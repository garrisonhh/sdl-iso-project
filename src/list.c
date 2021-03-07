#include <stdlib.h>
#include "list.h"

list_t *list_create() {
	list_t *list = (list_t *)malloc(sizeof(list_t));
	
	list->items = (void **)malloc(sizeof(void *) * 2);
	list->size = 0;
	list->max_size = 2;

	return list;
}

void list_destroy(list_t *list) {
	free(list->items);
	list->items = NULL;
	free(list);
	list = NULL;
}

void list_add(list_t *list, void *item) {
	list->items[list->size++] = item;

	if (list->size == list->max_size) {
		list->max_size <<= 1;
		list->items = (void **)realloc(list->items, sizeof(void *) * list->max_size);
	}
}

void list_remove(list_t *list, void *item) {
	int i, j;

	for (i = 0; i < list->size; i++) {
		if (list->items[i] == item) {
			for (j = i + 1; j < list->size; j++)
				list->items[j - 1] = list->items[j];
			list->size--;

			if (list->max_size > 2 && list->size < list->max_size >> 1) {
				list->max_size >>= 1;
				list->items = (void **)realloc(list->items, sizeof(void *) * list->max_size);
			}
			break;
		}
	}
}

