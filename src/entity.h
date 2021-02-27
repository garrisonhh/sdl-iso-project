#ifndef ENTITY_H
#define ENTITY_H

#include "vector.h"
#include "sprites.h"
#include "collision.h"

typedef struct {
	int sprite;
	v3d pos;
	v3d move; // speed is in blocks per second
	// TODO this should probably become a pointer
	// maybe load entity sprite/bbox data from a json?
	bbox_t bbox;
} entity_t;

void entity_destroy(entity_t *);
void entity_tick(entity_t *, int ms);
bbox_t entity_abs_bbox(entity_t *);

#endif
