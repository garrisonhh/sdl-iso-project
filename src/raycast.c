#include <stdbool.h>
#include <math.h>
#include "world.h"
#include "render.h"
#include "camera.h"
#include "collision.h"
#include "lib/vector.h"
#include "lib/utils.h"

// block testing
bool raycast_block_exists(block_t *block) {
	return block != NULL;
}

/*
 * finds location of first block hit and axis hit
 * if the ray starts inside of a block, axis will be -1
 * 
 * beware of submitting a ray which has a position where coordinates are close to int values,
 * this can cause floating point errors which make the algorithm inaccurate. unsure why.
 * **this does NOT limit itself to the scope of the ray**
 */
bool raycast_to_block(world_t *world, ray_t ray, bool (*test_block)(block_t *), v3i *block_hit, int *axis_hit) {
#define RAYCAST_RETURN {\
		if (block_hit != NULL)\
			*block_hit = loc;\
		if (axis_hit != NULL)\
			*axis_hit = axis;\
		return true;\
	}

	int i, axis;
	double t_max_target;
	double dim_pos, dim_abs_dir;
	double axis_diff;
	v3i loc, step;
	v3d t_max, t_delta;

	// find initial vars
	loc = v3i_from_v3d(ray.pos);
	step = polarity_of_v3d(ray.dir);

	for (i = 0; i < 3; ++i) {
		dim_pos = v3d_IDX(ray.pos, i);
		dim_abs_dir = fabs(v3d_IDX(ray.dir, i));

		// t_max: t where ray crosses boundary from initial position on each axis
		t_max_target = (v3i_IDX(step, i) > 0 ? ceil(dim_pos) : floor(dim_pos));

		v3d_IDX(t_max, i) = fabs(t_max_target - dim_pos) / dim_abs_dir;

		// t_delta: t where ray changes by 1 on each axis
		v3d_IDX(t_delta, i) = 1.0 / dim_abs_dir;
	}

	// offset loc for negative directions
	if (test_block(world_get(world, loc))) {
		axis = 2;
		RAYCAST_RETURN;
	}

	for (i = 0; i < 3; ++i)
		if (v3i_IDX(step, i) < 0)
			--v3i_IDX(loc, i);

	// iterate though voxel space
	while (true) {
		if (test_block(world_get(world, loc)))
			RAYCAST_RETURN;

		// find axis with minimum t
		axis = 2;

		for (i = 1; i >= 0; --i) {
			axis_diff = v3d_IDX(t_max, axis) - v3d_IDX(t_max, i);

			if (!d_close(axis_diff, 0.0) && axis_diff > 0.0)
				axis = i;
		}

		// increment loc by step and t_max by t_delta on axis
		v3i_IDX(loc, axis) += v3i_IDX(step, axis);

		if ((v3i_IDX(step, axis) < 0 && v3i_IDX(loc, axis) < 0)
		 || (v3i_IDX(step, axis) > 0 && v3i_IDX(loc, axis) >= world->block_size)) {
			return false;
		}

		v3d_IDX(t_max, axis) += v3d_IDX(t_delta, axis);
	}
}
