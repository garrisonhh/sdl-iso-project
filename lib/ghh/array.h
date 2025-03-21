#ifndef GHH_ARRAY_H
#define GHH_ARRAY_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ghh_array array_t;

// instancing
array_t *array_create(size_t initial_size);
void array_destroy(array_t *, bool destroy_values);

// data access
size_t array_size(array_t *);
void *array_get(array_t *, size_t index);
void *array_set(array_t *, size_t index, void *item); // returns old value
void *array_raw(array_t *); // don't use this unless you know what you're doin

// stack ops
void array_push(array_t *, void *item);
void *array_pop(array_t *);
void *array_peek(array_t *);

// array ops
void *array_del(array_t *, size_t index);
void array_clear(array_t *, bool destroy_values);
void array_qsort(array_t *, int (*compare)(const void *, const void *));

#define ARRAY_FOREACH(item, array)\
    for (\
        size_t ARR_FE_ITER = 0;\
        ARR_FE_ITER < array_size(array)\
            ? (item = array_get(array, ARR_FE_ITER), true)\
            : false;\
        ++ARR_FE_ITER\
    )

#endif
