#ifndef GHH_HASHMAP_H
#define GHH_HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct ghh_hashmap hashmap_t;
typedef struct ghh_hmapiter hmapiter_t;

// instancing
// key_size < 0: hashes as a string
// key_size > 0: hashes a type where key_size == sizeof(type)
// beware, if structs with padding have not been calloc'd or memset, key comparison is UB
hashmap_t *hashmap_create(size_t initial_size, int key_size, bool copy_keys);
void hashmap_destroy(hashmap_t *, bool destroy_values);

// data access
size_t hashmap_size(hashmap_t *);

// hashmap ops
void *hashmap_get(hashmap_t *, const void *key);
// returns true if bucket exists, whether or not value is NULL.
// more robust than `if ((value = hashmap_get()) != NULL) {}` idiom.
bool hashmap_may_get(hashmap_t *, const void *key, void **out_value);
void *hashmap_set(hashmap_t *, const void *key, const void *value);
void *hashmap_remove(hashmap_t *, const void *key);

// hash set-ish ops TODO hashset_t?
bool hashmap_has(hashmap_t *, const void *key);
#define hashmap_give(hmap, key) hashmap_set(hmap, key, NULL)

// hashmap iterator

/* usage:
hmapiter_t *iterator = hmapiter_create(hashmap);
void *value;

while (hmapiter_next(iterator, &value))
    ; //do stuff ...

free(iterator);
*/
// hmapiter_t is reset on stop, so can be reused as many times as you want
// hmapiter_t does not have to be recreated when hashmap is changed, but it
// will not work as intended if you edit hashmap during iteration.

hmapiter_t *hmapiter_create(hashmap_t *);
// free() to destroy

void hmapiter_reset(hmapiter_t *);
// returns whether not done
bool hmapiter_next(hmapiter_t *, void **out_key, void **out_value);

// these are convenient, but they hide a lot of functionality, use with caution
#define HMAP_FOREACH_KV(key, value, hmap) for (\
    hmapiter_t *HMAP_FE_ITER = hmapiter_create(hmap);\
    hmapiter_next(HMAP_FE_ITER, (void **)&key, (void **)&value)\
        ? true : (free(HMAP_FE_ITER), false);\
)

#define HMAP_FOREACH_K(key, hmap) for (\
    hmapiter_t *HMAP_FE_ITER = hmapiter_create(hmap);\
    hmapiter_next(HMAP_FE_ITER, (void **)&key, NULL) ? true : (free(HMAP_FE_ITER), false);\
)

#define HMAP_FOREACH_V(value, hmap) for (\
    hmapiter_t *HMAP_FE_ITER = hmapiter_create(hmap);\
    hmapiter_next(HMAP_FE_ITER, NULL, (void **)&value) ? true : (free(HMAP_FE_ITER), false);\
)

#endif
