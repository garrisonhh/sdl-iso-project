#include "entity.h"
#include "vector.h"
#include "collision.h"

entity_t *player_create() {
	entity_t *player;
	v3d pos, size;

	pos = (v3d){5.0, 5.0, 20.0};
	size = (v3d){0.5, 0.5, 1.0};
	player = entity_create(0, pos, size);

	return player;
}

