#ifndef ENTITY_HUMAN_H
#define ENTITY_HUMAN_H

#include "textures.h"
#include "animation.h"
#include "data_structures/array.h"

#define HUMAN_WALK_VELOCITY 5.0
#define HUMAN_JUMP_VELOCITY 7.5

struct tool_t {
	texture_t *sprites[2];
	animation_t anim_state;
};
typedef struct tool_t tool_t;

struct human_t {
	texture_t *hands[2];
	tool_t *tool;
};
typedef struct human_t human_t;

human_t *human_create(void);
void human_destroy(human_t *);

entity_t *entity_human_create();

void entity_human_tick(entity_t *, double time);

#endif
