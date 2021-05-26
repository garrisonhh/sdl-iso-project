#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdbool.h>
#include "vector.h"
#include "sprites.h"

#define ANIMATION_FPS 12.0

typedef struct entity_t entity_t;
typedef struct animation_t animation_t;

enum dir_xy_e {
	DIR_FRONT,
	DIR_BACK,
	DIR_LEFT,
	DIR_RIGHT,

	DIR_FRONT_LEFT,
	DIR_FRONT_RIGHT,
	DIR_BACK_LEFT,
	DIR_BACK_RIGHT,
};
typedef enum dir_xy_e dir_xy_e;

enum dir_z_e {
	DIR_UP,
	DIR_DOWN,
	DIR_LEVEL
};
typedef enum dir_z_e dir_z_e;

struct animation_t {
	v2i cell;
	double state;
	bool done;
};
typedef struct animation_t animation_t;

animation_t anim_empty_state(void);

void anim_entity_update_directions(entity_t *entity);
void anim_tick(entity_t *, sprite_t *, animation_t *, double time);

#endif
