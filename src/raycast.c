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

// finds location of first block hit and axis hit
// if the ray starts inside of a block, axis will be -1
// **this does NOT limit itself to the scope of the ray**
bool raycast_to_block(world_t *world, ray_t ray, bool (*test_block)(block_t *), v3i *block_hit, int *axis_hit) {
	int i, polarity, axis;
	double t_max_target, dim_pos, dim_abs_dir;
	v3i loc, step;
	v3d t_max, t_delta;

	// find initial vars
	axis = -1;
	loc = v3i_from_v3d(ray.pos);
	step = polarity_of_v3d(ray.dir);

	for (i = 0; i < 3; i++) {
		dim_pos = v3d_IDX(ray.pos, i);
		dim_abs_dir = fabs(v3d_IDX(ray.dir, i));
		polarity = v3i_IDX(step, i);

		// t_max: scalar t where ray crosses boundary from initial position on each axis
		t_max_target = (polarity > 0 ? ceil(dim_pos) : floor(dim_pos));

		if (d_close(dim_pos, t_max_target))
			t_max_target += polarity;

		v3d_IDX(t_max, i) = fabs(t_max_target - dim_pos) / dim_abs_dir;

		// t_delta: scalar t where ray changes by 1 on each axis
		v3d_IDX(t_delta, i) = 1.0 / dim_abs_dir;
	}

	// iterate though voxel space
	while (true) {
		if (test_block(world_get(world, loc))) {
			if (block_hit != NULL)
				*block_hit = loc;
			if (axis_hit != NULL)
				*axis_hit = axis;
			return true;
		}

		// find axis with minimum t
		axis = 2;

		for (i = 1; i >= 0; i--)
			if (v3d_IDX(t_max, i) < v3d_IDX(t_max, axis))
				axis = i;

		// increase by step and delta on axis
		v3i_IDX(loc, axis) += v3i_IDX(step, axis);

		if ((v3i_IDX(step, axis) < 0 && v3i_IDX(loc, axis) < 0)
		 || (v3i_IDX(step, axis) > 0 && v3i_IDX(loc, axis) >= world->block_size)) {
			return false;
		}

		v3d_IDX(t_max, axis) += v3d_IDX(t_delta, axis);
	}
}

bool raycast_screen_pos(world_t *world, v2i screen_pos, v3i *block_hit, int *axis_hit) {
	double a, b;
	ray_t ray;
	v2d cartesian;

	// reverse projection
	cartesian = v2d_from_v2i(screen_pos);
	cartesian.x = (cartesian.x / (double)camera.scale) + (double)camera.center.x;
	cartesian.y = (cartesian.y / (double)camera.scale) + (double)camera.center.y;

	a = (cartesian.x * 2.0) / (double)VOXEL_WIDTH;
	b = (cartesian.y * 4.0) / (double)VOXEL_WIDTH;

	ray.pos = (v3d){
		(b - a) * 0.5,
		(b + a) * 0.5,
		0.0
	};
	ray.dir = (v3d){0, 0, 0};
	ray.dir = v3d_sub(ray.dir, CAMERA_VIEW_DIR);

	// adjust ray to start at world block height
	// TODO ray interaction with fg/bg splitting?
	ray.pos = v3d_add(ray.pos, v3d_scale(ray.dir, (double)world->block_size / ray.dir.z));

	v3d_print("raycasting from", ray.pos);
	v3d_print("raycasting towards", ray.dir);

	return raycast_to_block(world, ray, raycast_block_exists, block_hit, axis_hit);
}
