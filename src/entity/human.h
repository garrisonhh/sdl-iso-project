#ifndef ENTITY_HUMAN_H
#define ENTITY_HUMAN_H

#include "entity.h"
#include "../sprites.h"
#include "../animation.h"
#include <ghh/list.h>

#define HUMAN_WALK_VELOCITY 5.0
#define HUMAN_JUMP_VELOCITY 7.5

/*
typedef struct tool {
	sprite_t *sprites[2];
} tool_t;
*/

human_t *human_create(void);

void human_tick(human_t *, double time);

#endif
