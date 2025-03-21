#ifndef GHH_LIST_H
#define GHH_LIST_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ghh_list list_t;
typedef struct ghh_listiter listiter_t;

// list
list_t *list_create(void);
void list_destroy(list_t *, bool destroy_values);

// data access
size_t list_size(list_t *);
void *list_get(list_t *, size_t index);
void *list_get_root(list_t *);
void *list_get_tip(list_t *);

// stack/queue ops
void list_push(list_t *, void *item); // item + [:]
void list_append(list_t *, void *item); // [:] + item
void *list_pop(list_t *);
void *list_peek(list_t *);

// list ops
void list_insert(list_t *, size_t index, void *item); // [:index] + item + [index:]
// directly compares pointers
void *list_remove(list_t *, void *item);
void list_merge(list_t *, list_t *other);

// list iterator (use LIST_FOREACH if you don't need any extra functionality)
listiter_t *listiter_create(list_t *);

void listiter_reset(listiter_t *);
// returns whether not done
bool listiter_next(listiter_t *, void **out_value);

// this is a disgusting macro
#define LIST_FOREACH(item, list) for (\
    listiter_t *LIST_FE_ITER = listiter_create(list);\
    listiter_next(LIST_FE_ITER, (void **)&item) ? true : (free(LIST_FE_ITER), false);\
)

#endif
