#ifndef ENTITY_COLLISION_H
#define ENTITY_COLLISION_H

#include "entity.h"
#include "../world.h"

void entity_move_and_collide(entity_t *, world_t *world, double time);

#endif
