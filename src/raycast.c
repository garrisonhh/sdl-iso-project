#include <stdbool.h>
#include <math.h>
#include "raycast.h"
#include "lib/utils.h"

// TODO skip over empty chunks
bool raycast_voxels(world_t *world, ray_t ray, v3i *out_block, int *out_axis) {
	int i;
	bool P = out_axis != NULL;

	// initialization step
	double cur_index;
	v3i loc = v3i_from_v3d(ray.pos);
	v3i step = polarity_of_v3d(ray.dir);
	v3i end;
	v3d t_max, t_delta;

	for (i = 0; i < 3; ++i) {
		if (v3i_IDX(step, i) > 0) {
			cur_index = ceil(v3d_IDX(ray.pos, i));
		} else {
			cur_index = floor(v3d_IDX(ray.pos, i));
			--v3i_IDX(loc, i);
		}

		v3d_IDX(t_max, i) = (cur_index - v3d_IDX(ray.pos, i)) / v3d_IDX(ray.dir, i);
		v3d_IDX(t_delta, i) = 1.0 / v3d_IDX(ray.dir, i);
		v3d_IDX(t_max, i) = fabs(v3d_IDX(t_max, i));
		v3d_IDX(t_delta, i) = fabs(v3d_IDX(t_delta, i));

		v3i_IDX(end, i) = (v3i_IDX(step, i) > 0 ? world->block_size : -1);
	}

	if (P) {
		v3i_print("loc", loc);
		v3i_print("step", step);
		v3i_print("end", end);
		v3d_print("t_max", t_max);
		v3d_print("t_delta", t_delta);
	}

	// incrementation step
	int axis;

	do {
		if (P) v3i_print(NULL, loc);

		// find axis with minimum t_max
		axis = 0;

		for (i = 1; i < 3; ++i)
			if (v3d_IDX(t_max, i) < v3d_IDX(t_max, axis))
				axis = i;

		// increment on axis
		v3i_IDX(loc, axis) += v3i_IDX(step, axis);

		if (v3i_IDX(loc, axis) == v3i_IDX(end, axis)) {
			if (P) printf("out of bounds\n");

			return false;
		}

		v3d_IDX(t_max, axis) += v3d_IDX(t_delta, axis);
	} while (world_get(world, loc) == NULL);

	if (P) v3i_print("hit block", loc);

	// block hit, return it
	if (out_block != NULL)
		*out_block = loc;

	if (out_axis != NULL)
		*out_axis = axis;

	return true;
}
