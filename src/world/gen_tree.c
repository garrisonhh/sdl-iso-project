#include <stdlib.h>
#include "gen_tree.h"
#include "../world.h"
#include "../collision.h"
#include "../block/blocks.h"
#include "../lib/utils.h"

l_system_t *tree_gen_l_system_create() {
	l_system_t *lsys = l_system_create("");

	return lsys;
}

void tree_gen_draw_branch(world_t *world, ray_t branch, double radius) {
	const v3d BLOCK_CENTER = {0.5, 0.5, 0.5};

	const BLOCK_DECL(dirt);

	v3i start, end, loc;
	double v_min, v_max, swp;
	sphere_t sphere;

	sphere.radius = radius;

	for (int i = 0; i < 3; ++i) {
		// get min/max vals for axis
		v_min = v3d_IDX(branch.pos, i);
		v_max = v_min + v3d_IDX(branch.dir, i);

		if (v_min > v_max)
			SWAP(v_min, v_max, swp);

		v_min -= radius;
		v_max += radius;

		// set start/end
		v3i_IDX(start, i) = (int)v_min - 1;
		v3i_IDX(end, i) = (int)v_max + 1;
	}

	for (loc.z = start.z; loc.z <= end.z; ++loc.z) {
		for (loc.y = start.y; loc.y <= end.y; ++loc.y) {
			for (loc.x = start.x; loc.x <= end.x; ++loc.x) {
				sphere.pos = v3d_add(v3d_from_v3i(loc), BLOCK_CENTER);

				if (ray_intersects_sphere(branch, sphere, NULL))
					world_set_no_update(world, loc, dirt);
			}
		}
	}
}

void tree_generate(world_t *world, l_system_t *lsys, v3i loc, int iterations) {
	ray_t ray = {
		.pos = v3d_from_v3i(loc),
		.dir = (v3d){
			(randf() - 0.5) * 10.0,
			(randf() - 0.5) * 10.0,
			20.0
		}
	};

	tree_gen_draw_branch(world, ray, 1.5);
}
