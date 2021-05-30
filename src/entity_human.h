#ifndef ENTITY_HUMAN_H
#define ENTITY_HUMAN_H

#include "sprites.h"
#include "animation.h"
#include "lib/array.h"

#define HUMAN_WALK_VELOCITY 5.0
#define HUMAN_JUMP_VELOCITY 7.5

struct tool_t {
	sprite_t *sprites[2];
};
typedef struct tool_t tool_t;

struct human_t {
	sprite_t *hands[2];
	tool_t *tool;
	animation_t anim_state; // shared between both tool/hand sprites
	bool using_tool;
};
typedef struct human_t human_t;

human_t *human_create(void);
void human_destroy(human_t *);

entity_t *entity_human_create();

void entity_human_tick(entity_t *, double time);

void entity_human_use_tool(entity_t *);

#endif
