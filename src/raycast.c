#include <stdbool.h>
#include <math.h>
#include "raycast.h"
#include <ghh/utils.h>

// TODO skip over empty chunks
bool raycast_voxels(world_t *world, ray_t ray, v3i *out_block, int *out_axis) {
	int i;

	// initialization step
	double cur_index;
	v3i loc, end;
	v3i step = polarity_of_v3d(ray.dir);
	v3d t_max, t_delta;

	for (i = 0; i < 3; ++i) {
		if (v3i_IDX(step, i) < 0) {
			--v3d_IDX(ray.pos, i);
			cur_index = floor(v3d_IDX(ray.pos, i));
		} else {
			cur_index = ceil(v3d_IDX(ray.pos, i));
		}

		v3d_IDX(t_max, i) = (cur_index - v3d_IDX(ray.pos, i)) / v3d_IDX(ray.dir, i);
		v3d_IDX(t_delta, i) = 1.0 / v3d_IDX(ray.dir, i);
		v3d_IDX(t_max, i) = fabs(v3d_IDX(t_max, i));
		v3d_IDX(t_delta, i) = fabs(v3d_IDX(t_delta, i));

		v3i_IDX(loc, i) = (int)v3d_IDX(ray.pos, i);
		v3i_IDX(end, i) = (v3i_IDX(step, i) > 0 ? world->block_size : -1);
	}

	// incrementation step
	int axis;

	do {
		// find axis with minimum t_max
		axis = 0;

		for (i = 1; i < 3; ++i)
			if (v3d_IDX(t_max, i) < v3d_IDX(t_max, axis))
				axis = i;

		// increment loc and t_max on axis
		v3i_IDX(loc, axis) += v3i_IDX(step, axis);

		if (v3i_IDX(loc, axis) == v3i_IDX(end, axis))
			return false;

		v3d_IDX(t_max, axis) += v3d_IDX(t_delta, axis);
	} while (world_get(world, loc) == NULL);

	// block hit, return it
	if (out_block != NULL)
		*out_block = loc;

	if (out_axis != NULL)
		*out_axis = axis;

	return true;
}
