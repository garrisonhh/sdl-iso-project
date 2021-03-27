#ifndef ENTITY_BUCKET_H
#define ENTITY_BUCKET_H

#include <stdlib.h>

struct list_t {
	void **items;
	size_t size, max_size;
};
typedef struct list_t list_t;

list_t *list_create(void);
void list_destroy(list_t *);
void list_deep_destroy(list_t *);
void list_add(list_t *, void *);
void list_remove(list_t *, void *);
void list_qsort(list_t *, int (*)(const void *, const void *));

#endif
