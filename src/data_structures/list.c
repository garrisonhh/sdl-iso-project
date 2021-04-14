#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "list.h"

list_t *list_create() {
	list_t *list = (list_t *)malloc(sizeof(list_t));

	list->root = NULL;
	list->tip = NULL;
	list->size = 0;

	return list;
}

void list_destroy(list_t *list, bool destroy_values) {
	list_node_t *trav, *last;

	trav = list->root;

	while (trav != NULL) {
		last = trav;
		trav = trav->next;

		if (destroy_values)
			free(last->item);

		free(last);
	}

	free(list);
}

void list_push(list_t *list, void *item) {
	list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));

	node->item = item;

	if (list->root == NULL) {
		node->next = NULL;
		list->root = node;
		list->tip = node;
	} else {
		node->next = list->root;
		list->root = node;
	}

	++list->size;
}

void *list_pop(list_t *list) {
	if (list->root == NULL) {
		printf("attempted to pop from empty list.\n");
		exit(1);
	}

	list_node_t *old_root;
	void *item;

	old_root = list->root;
	item = old_root->item;
	list->root = old_root->next;

	free(old_root);

	if (--list->size == 0)
		list->tip = NULL;

	return item;
}

void *list_peek(list_t *list) {
	return list->root->item;
}

void list_append(list_t *list, void *item) {
	list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));

	node->item = item;
	node->next = NULL;

	if (list->size) {
		list->tip->next = node;
		list->tip = node;
	} else {
		list->root = node;
		list->tip = node;
	}

	++list->size;
}

void *list_get(list_t *list, size_t index) {
	if (index >= list->size) {
		printf("tried to access node at index outside of linked list.\n");
		exit(1);
	}

	int i = 0;
	list_node_t *trav = list->root;

	while (++i < index)
		trav = trav->next;

	return trav;
}

// list_set
// list_delete

void *list_remove(list_t *list, void *item) {
	list_node_t *trav, *last;

	last = NULL;
	trav = list->root;
	
	while (trav != NULL && trav->item != item) {
		last = trav;
		trav = trav->next;
	}

	if (trav != NULL) {
		void *item;

		if (trav == list->root)
			list->root = trav->next;
		else
			last->next = trav->next;

		item = trav->item;
		free(trav);
		--list->size;

		return item;
	}

	return NULL;
}

void list_merge(list_t *list, list_t *other) {
	if (list->size) {
		list->tip->next = other->root;
		list->tip = other->tip;
	} else {
		list->root = other->root;
		list->tip = other->tip;
	}

	list->size += other->size;

	other->root = NULL;
	other->tip = NULL;
	other->size = 0;
}
