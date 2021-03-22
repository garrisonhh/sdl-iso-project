#include <stdbool.h>
#include <math.h>
#include "world.h"
#include "vector.h"
#include "collision.h"
#include "utils.h"

void raycast_ray_data(ray_t ray, v3i *loc, v3i *step, v3d *t_max, v3d *t_delta) {
	double t_max_target, dim_pos, dim_abs_dir;
	int i;

	*loc = v3i_from_v3d(ray.pos);
	*step = polarity_of_v3d(ray.dir);

	for (i = 0; i < 3; i++) {
		dim_pos = v3d_get(&ray.pos, i);
		dim_abs_dir = fabs(v3d_get(&ray.dir, i));

		// t_max: scalar t where ray crosses boundary from initial position on each axis
		t_max_target = (v3i_get(step, i) > 0 ? ceil(dim_pos) : floor(dim_pos));
		v3d_set(t_max, i, (fabs(t_max_target - dim_pos) / dim_abs_dir));

		// t_delta: scalar t where ray changes by 1 on each axis
		v3d_set(t_delta, i, 1.0 / dim_abs_dir);
	}
}

// finds location of first block hit and axis hit
// if the ray starts inside of a block, axis will be -1
// implements the algorithm described in doi=10.1.1.42.3443
// **this does NOT limit itself to the scope of the ray**
bool raycast_to_block(world_t *world, ray_t ray, v3i *block_hit, int *axis_hit) {
	int i, axis, axis_next_val;
	v3i loc, step;
	v3d t_max, t_delta;

	raycast_ray_data(ray, &loc, &step, &t_max, &t_delta);
	axis = -1;

	while (true) {
		if (block_get(world, loc) != NULL) {
			if (block_hit != NULL)
				*block_hit = loc;
			if (axis_hit != NULL)
				*axis_hit = axis;
			return true;
		}

		// find axis with minimum t
		axis = 2;

		for (i = 1; i >= 0; i--)
			if (v3d_get(&t_max, i) < v3d_get(&t_max, axis))
				axis = i;

		// increase by step and delta on axis
		axis_next_val = v3i_get(&loc, axis) + v3i_get(&step, axis);
		v3i_set(&loc, axis, axis_next_val);

		if (axis_next_val < 0 || axis_next_val >= world->block_size)
			return false;

		v3d_set(&t_max, axis, v3d_get(&t_max, axis) + v3d_get(&t_delta, axis));
	}
}

// TODO line of sight between two v3ds for pathfinding
