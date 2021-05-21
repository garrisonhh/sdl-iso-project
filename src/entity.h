#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include <stdbool.h>
#include "data_structures/list.h"
#include "data_structures/array.h"
#include "collision.h"
#include "textures.h"
#include "animation.h"
#include "entity_human.h"

typedef struct world_t world_t;

enum entity_type_e {
	ENTITY_BASE,
	ENTITY_HUMAN,
};
typedef enum entity_type_e entity_type_e;

// "entity" in this context basically means a physics object with a sprite
struct entity_t {
	// state
	entity_type_e type;
	union entity_state {
		human_t *human;
	} state;

	// sprite + animation
	texture_t *sprite;
	animation_t anim_state;

	v3d last_dir;
	dir_xy_e dir_xy;
	dir_z_e dir_z;

	// collision
	ray_t ray;
	v3d size, center;

	bool on_ground;
};
typedef struct entity_t entity_t;

entity_t *entity_create(entity_type_e type, texture_t *sprites, v3d size);
void entity_destroy(entity_t *);

void entity_tick(entity_t *, world_t *, double time);

v2i entity_screen_pos(entity_t *entity);
void entity_add_render_packets(entity_t *entity, array_t *packets);

#endif
