#ifndef RAYCAST_H
#define RAYCAST_H

#include "world.h"
#include "vector.h"
#include "collision.h"

bool raycast_to_block(world_t *, ray_t, v3i *);

#endif
