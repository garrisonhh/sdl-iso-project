#ifndef ANIMATION_H
#define ANIMATION_H

#include "vector.h"
#include "textures.h"

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
};
typedef struct animation_t animation_t;

void anim_entity_update_directions(entity_t *entity);
void anim_tick(entity_t *, texture_t *, animation_t *, double time);

#endif
