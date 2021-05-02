#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

#define LIST_FOREACH(node, list) for (node = list->root; node != NULL; node = node->next)

struct list_node_t {
	void *item;
	struct list_node_t *next;
};
typedef struct list_node_t list_node_t;

struct list_t {
	list_node_t *root, *tip;
	size_t size;
};
typedef struct list_t list_t;

list_t *list_create();
void list_destroy(list_t *, bool destroy_values);

void list_push(list_t *, void *item);
void *list_pop(list_t *);
void *list_peek(list_t *);
void list_append(list_t *, void *item);

void *list_get(list_t *, size_t index);

// directly compares pointers
void *list_remove(list_t *, void *item);
void list_merge(list_t *, list_t *other);

#endif
