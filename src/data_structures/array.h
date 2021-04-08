#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <stdlib.h>

typedef struct array_t array_t;

array_t *array_create(void);
void array_destroy(array_t *, bool destroy_values);

size_t array_size(array_t *);
void array_add(array_t *, void *item);
void array_remove(array_t *, void *item);
void array_qsort(array_t *, int (*compare)(const void *, const void *));

#endif
