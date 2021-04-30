#ifndef RAYCAST_H
#define RAYCAST_H

#include "world.h"
#include "vector.h"
#include "collision.h"

bool raycast_block_exists(block_t *);

bool raycast_to_block(world_t *, ray_t, bool (*block_test)(block_t *),
					  v3i *block_hit, int *axis_hit);

#endif
