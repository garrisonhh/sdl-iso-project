#ifndef ENTITY_BUCKET_H
#define ENTITY_BUCKET_H

#include "entity.h"

typedef struct {
	entity_t **arr;
	int size, max_size;
} entity_bucket;

entity_bucket *bucket_create(void);
void bucket_destroy(entity_bucket *);
void bucket_add(entity_bucket *, entity_t *);
void bucket_remove(entity_bucket *, entity_t *);

#endif
