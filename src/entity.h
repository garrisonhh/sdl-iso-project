#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include "collision.h"
#include "textures.h"
#include "data_structures/list.h"

typedef struct world_t world_t;

struct entity_t {
	// sprite
	sprite_t *sprite;
	v2i anim_cell;
	double anim_state;

	// collision
	ray_t ray;
	v3d size, center;

	// pathing
	bool on_ground;
	list_t *path;
};
typedef struct entity_t entity_t;

entity_t *entity_create(sprite_t *, v3d pos, v3d size);

void entity_destroy(entity_t *);
void entity_add_path(entity_t *, list_t *);
void entity_tick(entity_t *, world_t *, double time);

#endif
