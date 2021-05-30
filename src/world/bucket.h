#ifndef WORLD_BUCKET_H
#define WORLD_BUCKET_H

#include "../entity.h"
#include "../lib/vector.h"
#include "../lib/list.h"

typedef struct world_t world_t;
void world_bucket_add(world_t *world, v3i loc, entity_t *entity);

// render y sorting
double world_bucket_y(entity_t *);
int world_bucket_compare(const void *, const void *);

void world_bucket_remove(world_t *world, v3i loc, entity_t *entity);
void world_bucket_z_sort(list_t *bucket);

#endif
