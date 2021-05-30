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

typedef struct world_t world_t;

enum entity_type_e {
	ENTITY_BASE,
	ENTITY_HUMAN,
};
typedef enum entity_type_e entity_type_e;

struct entity_data_t {
	entity_type_e type;

	// sprite + animation
	sprite_t *sprite;
	animation_t anim_state;

	v3d last_dir;
	dir_xy_e dir_xy;
	dir_z_e dir_z;

	// physics
	ray_t ray;
	v3d size, center;
	bool on_ground; // for gravity application
};
typedef struct entity_data_t entity_data_t;

struct human_t {
	entity_data_t _data;

	sprite_t *hands[2];
	animation_t anim_state;
};
typedef struct human_t human_t;

union entity_t {
	entity_data_t data;
	human_t human;
};
typedef union entity_t entity_t;

void entity_destroy(entity_t *);
void entity_data_populate(entity_t *, entity_type_e, sprite_t *, v3d size);

void entity_tick(entity_t *, world_t *, double time);

void entity_add_render_info(array_t *packets, entity_t *);

#endif
