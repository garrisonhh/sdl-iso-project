#define GHH_LIB_INTERNAL
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ghh/list.h>
#include <ghh/memcheck.h>
#include <ghh/utils.h>

typedef struct ghh_listnode {
	void *item;
	struct ghh_listnode *next;
} listnode_t;

struct ghh_list {
	listnode_t *root, *tip;
	size_t size;
};

struct ghh_listiter {
	list_t *list;
	listnode_t *node;
};

list_t *list_create() {
	list_t *list = malloc(sizeof(*list));

	list->root = NULL;
	list->tip = NULL;
	list->size = 0;

	return list;
}

void list_destroy(list_t *list, bool destroy_values) {
	listnode_t *trav, *last;

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

size_t list_size(list_t *list) {
	return list->size;
}

void *list_get_root(list_t *list) {
	return list->root->item;
}

void *list_get_tip(list_t *list) {
	return list->tip->item;
}

void *list_get(list_t *list, size_t index) {
#ifdef DEBUG
	if (index >= list->size)
		ERROR("tried to access node at index outside of linked list.\n");
#endif

	int i = 0;
	listnode_t *trav = list->root;

	while (++i < index)
		trav = trav->next;

	return trav;
}

void list_insert(list_t *list, size_t index, void *item) {
	if (index >= list->size) {
		list_append(list, item);
	} else if (index == 0) {
		list_push(list, item);
	} else {
		int i = 0;
		listnode_t *trav = list->root;
		listnode_t *node = malloc(sizeof(*node));

		node->item = item;

		while (++i < index)
			trav = trav->next;

		node->next = trav->next;
		trav->next = node;

		++list->size;
	}
}

void list_push(list_t *list, void *item) {
	listnode_t *node = malloc(sizeof(*node));

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

void list_append(list_t *list, void *item) {
	listnode_t *node = malloc(sizeof(*node));

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

void *list_pop(list_t *list) {
#ifdef DEBUG
	if (!list->root)
		ERROR("attempted to pop from empty list.\n");
#endif

	listnode_t *old_root;
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

void *list_remove(list_t *list, void *item) {
	listnode_t *trav, *last;

	last = NULL;
	trav = list->root;

	while (trav != NULL && trav->item != item) {
		last = trav;
		trav = trav->next;
	}

	if (trav != NULL) {
		void *removed;

		if (trav == list->root) {
			list->root = trav->next;
		} else if (trav == list->tip) {
			list->tip = last;
			last->next = NULL;
		} else {
			last->next = trav->next;
		}

		removed = trav->item;
		free(trav);
		--list->size;

		return removed;
	}

	return NULL;
}

void list_merge(list_t *list, list_t *other) {
	if (list->size)
		list->tip->next = other->root;
	else
		list->root = other->root;

	list->tip = other->tip;
	list->size += other->size;

	other->root = NULL;
	other->tip = NULL;
	other->size = 0;
}

listiter_t *listiter_create(list_t *list) {
	listiter_t *iter = malloc(sizeof(*iter));

	iter->list = list;
	iter->node = NULL;

	return iter;
}

void list_iter_reset(listiter_t *iter) {
	iter->node = NULL;
}

bool listiter_next(listiter_t *iter, void **out_value) {
	if (iter->node == NULL)
		iter->node = iter->list->root;
	else
		iter->node = iter->node->next;

	if (iter->node == NULL) {
		if (out_value != NULL)
			*out_value = NULL;

		return false;
	} else {
		if (out_value != NULL)
			*out_value = iter->node->item;

		return true;
	}
}
