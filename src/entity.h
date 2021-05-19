#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include <stdbool.h>
#include "collision.h"
#include "textures.h"
#include "animation.h"
#include "data_structures/list.h"
#include "entity_human.h"

typedef struct world_t world_t;

enum entity_type_e {
	ENTITY_BASE,
	ENTITY_HUMAN,
};
typedef enum entity_type_e entity_type_e;

/*
 * "entity" in this context basically means a physics object with a sprite
 */
struct entity_t {
	entity_type_e type;
	union typed_state {
		human_t *human;
	} state;

	// sprites + animations
	texture_t **sprites;
	animation_t *anim_states;
	size_t num_sprites;

	v3d last_dir;
	dir_xy_e dir_xy;
	dir_z_e dir_z;

	// collision
	ray_t ray;
	v3d size, center;

	bool on_ground; // TODO should this be part of a typed_state struct?
};
typedef struct entity_t entity_t;

entity_t *entity_create(entity_type_e type, texture_t **sprites, size_t num_sprites, v3d pos, v3d size);
void entity_destroy(entity_t *);

void entity_add_path(entity_t *, list_t *);
void entity_tick(entity_t *, world_t *, double time);

#endif
