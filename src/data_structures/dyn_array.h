#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stdlib.h>

struct dyn_array_t {
	void **items;
	size_t size, max_size;
};
typedef struct dyn_array_t dyn_array_t;

dyn_array_t *dyn_array_create(void);
void dyn_array_destroy(dyn_array_t *);
void dyn_array_deep_destroy(dyn_array_t *);
void dyn_array_add(dyn_array_t *, void *);
void dyn_array_remove(dyn_array_t *, void *);
void dyn_array_qsort(dyn_array_t *, int (*)(const void *, const void *));

#endif
