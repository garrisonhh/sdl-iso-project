#ifndef ENTITY_BUCKET_H
#define ENTITY_BUCKET_H

struct list_t {
	void **items;
	int size, max_size;
};
typedef struct list_t list_t;

list_t *list_create(void);
void list_destroy(list_t *);
void list_add(list_t *, void *);
void list_remove(list_t *, void *);

#endif
