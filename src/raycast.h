#ifndef RAYCAST_H
#define RAYCAST_H

#include "world.h"
#include "collision.h"
#include "lib/vector.h"

bool raycast_voxels(world_t *world, ray_t ray, v3i *out_block, int *out_axis);

#endif
