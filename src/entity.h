#ifndef ENTITY_H
#define ENTITY_H

#include "collision.h"

typedef struct {
	int sprite;
	ray_t ray;
	v3d size;
} entity_t;

void entity_destroy(entity_t *);
void entity_tick(entity_t *, int ms, bbox_t *, int);

#endif
