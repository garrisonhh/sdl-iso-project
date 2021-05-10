#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include <stdbool.h>
#include "collision.h"
#include "textures.h"
#include "animation.h"
#include "data_structures/list.h"

typedef struct world_t world_t;

struct entity_t {
	// sprites + animations
	sprite_t **sprites;
	size_t num_sprites;

	animation_t *anim_states;
	v3d last_dir;
	dir_xy_e dir_xy;
	dir_z_e dir_z;

	// collision
	ray_t ray;
	v3d size, center;

	// pathing
	bool on_ground;
	list_t *path;
};
typedef struct entity_t entity_t;

entity_t *entity_create(sprite_t **sprites, size_t num_sprites, v3d pos, v3d size);
void entity_destroy(entity_t *);

// entity bucket sorting
double entity_y(entity_t *);
int entity_bucket_compare(const void *, const void *);

void entity_add_path(entity_t *, list_t *);
void entity_tick(entity_t *, world_t *, double time);

#endif
