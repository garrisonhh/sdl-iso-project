#ifndef ENTITY_H
#define ENTITY_H

#include "vector.h"
#include "sprites.h"
#include "collision.h"

typedef struct {
	dvector3 pos;
	int sprite;
	// TODO this should probably become a pointer
	// maybe load different entities from a json?
	bbox_t bbox; 
} entity_t;

void destroyEntity(entity_t *);

#endif
