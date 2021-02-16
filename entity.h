#include "vector.h"
#include "sprites.h"

#ifndef ENTITY_H
#define ENTITY_H

typedef struct {
	dvector3 pos;
	int sprite;
} entity_t;

void destroyEntity(entity_t *);

#endif
