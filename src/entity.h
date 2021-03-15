#ifndef ENTITY_H
#define ENTITY_H

#include "collision.h"
#include "world.h"

/*
 * different entity types; e.g. 'creature', 'inanimate', etc.?
 */
struct entity_t {
	int sprite;
	ray_t ray;
	v3d size;
};
typedef struct entity_t entity_t;

void entity_destroy(entity_t *);
void entity_tick(entity_t *, int ms, bbox_t *, int);

#endif
