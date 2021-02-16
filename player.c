#include "entity.h"

entity_t *createPlayer() {
	entity_t *newPlayer = (entity_t *)malloc(sizeof(entity_t));
	newPlayer->sprite = 0;
	newPlayer->pos = (dvector3){0, 0, 0};

	return newPlayer;
}
