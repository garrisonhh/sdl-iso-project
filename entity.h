#ifndef ENTITY_H
#define ENTITY_H

#include "vector.h"
#include "sprites.h"
#include "collision.h"

typedef struct {
	int sprite;
	dvector3 pos;
	dvector3 move; // speed is in blocks per second
	// TODO this should probably become a pointer
	// maybe load entity sprite/bbox data from a json?
	bbox_t bbox;
} entity_t;

void destroyEntity(entity_t *);
void tickEntity(entity_t *, int ms);

#endif
