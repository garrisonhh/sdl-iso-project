#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include "collision.h"
#include "world.h"
#include "textures.h"
#include "data_structures/list.h"

struct world_t; // forward declaration to avoid header hell

struct entity_t {
	texture_t *sprite;
	ray_t ray;
	v3d size, center;

	bool on_ground;
	list_t *path;
};
typedef struct entity_t entity_t;

entity_t *entity_create(texture_t *, v3d pos, v3d size);
void entity_destroy(entity_t *);
void entity_add_path(entity_t *, list_t *);
void entity_tick(entity_t *, struct world_t *, double time);

#endif
