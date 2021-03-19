#ifndef ENTITY_H
#define ENTITY_H

#include "collision.h"
#include "world.h"

struct world_t; // forward declaration to avoid header hell

/*
 * different entity types; e.g. 'creature', 'inanimate', etc.?
 */
struct entity_t {
	int sprite;
	ray_t ray;
	v3d size, center;
};
typedef struct entity_t entity_t;

void entity_destroy(entity_t *);
void entity_tick(entity_t *, struct world_t *, int ms);

#endif
