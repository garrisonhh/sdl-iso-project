#include "entity.h"
#include "entity_human.h"
#include "vector.h"
#include "collision.h"
#include "textures.h"

entity_t *player_create() {
	v3d pos = (v3d){5.0, 5.0, 15.0};

	return entity_human_create(pos);
}

