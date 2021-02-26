#include "entity.h"
#include "vector.h"
#include "collision.h"

entity_t *createPlayer() {
	entity_t *newPlayer = (entity_t *)malloc(sizeof(entity_t));
	newPlayer->sprite = 0;
	newPlayer->pos = (v3d){5, 5, 15};
	newPlayer->move = (v3d){0, 0, 0};
	newPlayer->bbox = (bbox_t){(v3d){-.25, -.25, 0}, (v3d){.5, .5, 1}};

	return newPlayer;
}

