#ifndef ENTITY_H
#define ENTITY_H

#include <stdlib.h>
#include <stdbool.h>
#include "../lib/list.h"
#include "../lib/array.h"
#include "../render.h"
#include "../collision.h"
#include "../sprites.h"
#include "../animation.h"
#include "human.h"

typedef struct world_t world_t;

enum entity_type_e {
	ENTITY_BASE,
	ENTITY_HUMAN,

	NUM_ENTITY_TYPES
};
typedef enum entity_type_e entity_type_e;

struct entity_data_t {
	// state TODO move
	entity_type_e type;
	union entity_state {
		human_t *human;
	} state;

	// sprite + animation
	sprite_t *sprite;
	animation_t anim_state;

	v3d last_dir;
	dir_xy_e dir_xy;
	dir_z_e dir_z;

	// collision
	ray_t ray;
	v3d size, center;
	bool on_ground;
};
typedef struct entity_data_t entity_data_t;

// an entity is a physics object with an animatable sprite
union entity_t {
	entity_data_t data;
};
typedef union entity_t entity_t;

entity_t *entity_create(entity_type_e type, sprite_t *sprites, v3d size);
void entity_destroy(entity_t *);

void entity_tick(entity_t *, world_t *, double time);

#endif
