#include "entity.h"
#include "vector.h"
#include "collision.h"
#include "textures.h"

entity_t *player_create() {
	entity_t *player;
	v3d pos, size;

	pos = (v3d){5.0, 5.0, 31.0};
	size = (v3d){0.5, 0.5, 1.0};
	player = entity_create(texture_index("banana man"), pos, size);

	return player;
}

