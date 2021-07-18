#include <stdlib.h>
#include <ghh/utils.h>
#include "gen_tree.h"
#include "../world.h"
#include "../collision.h"
#include "../block/blocks.h"
#include "../lib/quaternion.h"

// TODO tree turtle is a hilarious idea for an animal in the game
typedef struct treeurtle {
	ray_t ray;
	double radius;
} tree_turtle_t;

typedef struct treeurtle_ctx {
	array_t *stack;
	tree_turtle_t turtle;
	double angle;
} tree_turtle_ctx_t;

struct tree_generator {
	l_system_t *lsys;
	int iterations;
	double branch_length, branch_radius;
	double size_persistence;
}; // typedef'd in header

void tree_turtle_branch(world_t *world, tree_turtle_ctx_t *ctx) {
	BLOCK_DECL(dirt); // TODO block for logs

	v3i start, end, loc;
	double v_min, v_max, swp;
	sphere_t sphere;

	sphere.radius = ctx->turtle.radius;

	// get range to iterate over
	for (int i = 0; i < 3; ++i) {
		// get min/max vals for axis
		v_min = v3d_IDX(ctx->turtle.ray.pos, i);
		v_max = v_min + v3d_IDX(ctx->turtle.ray.dir, i);

		if (v_min > v_max)
			SWAP(v_min, v_max, swp);

		v_min -= ctx->turtle.radius;
		v_max += ctx->turtle.radius;

		// set start/end
		v3i_IDX(start, i) = (int)(v_min - 2.0);
		v3i_IDX(end, i) = (int)(v_max + 2.0);
	}

	// iterate, checking whether point is close enough to ray
	for (loc.z = start.z; loc.z <= end.z; ++loc.z) {
		for (loc.y = start.y; loc.y <= end.y; ++loc.y) {
			for (loc.x = start.x; loc.x <= end.x; ++loc.x) {
				sphere.pos = v3d_add(v3d_from_v3i(loc), BLOCK_CENTER);

				if (ray_intersects_sphere(ctx->turtle.ray, sphere, NULL))
					world_set_no_update(world, loc, dirt);
			}
		}
	}
}

void tree_turtle_leaves(world_t *world, tree_turtle_ctx_t *ctx) {
	const double radius = ctx->turtle.radius * 6.0;
	const v3d turtle_pos = ctx->turtle.ray.pos;
	BLOCK_DECL(leaves);

	v3i start, end, loc;
	v3d pos;

	for (int i = 0; i < 3; ++i) {
		v3i_IDX(start, i) = (int)(v3d_IDX(turtle_pos, i) - radius) - 2;
		v3i_IDX(end, i) = (int)(v3d_IDX(turtle_pos, i) + radius) + 2;
	}

	for (loc.z = start.z; loc.z <= end.z; ++loc.z) {
		for (loc.y = start.y; loc.y <= end.y; ++loc.y) {
			for (loc.x = start.x; loc.x <= end.x; ++loc.x) {
				pos = v3d_add(v3d_from_v3i(loc), BLOCK_CENTER);

				if (v3d_dist(turtle_pos, pos) < radius && world_get(world, loc) == NULL)
					world_set_no_update(world, loc, leaves);
			}
		}
	}
}

quat_t tree_turtle_next_rotation(tree_turtle_ctx_t *ctx, double away_angle) {
	double rot_angle = ctx->angle;

	ctx->angle += (0.33 * M_PI * 2.0) + (rand_angle() / 3.0);

	// to rotate by a and then b, rotate by ba: (ba)p(ba)^(-1) = p'
	return quat_hamilton(quat_angle_rotation(rot_angle, UNIT_K),
						 quat_angle_rotation(away_angle, UNIT_I));
}

void tree_turtle_push(tree_turtle_ctx_t *ctx) {
	tree_turtle_t *state = malloc(sizeof(tree_turtle_t));

	*state = ctx->turtle;

	array_push(ctx->stack, state);
}

void tree_turtle_pop(tree_turtle_ctx_t *ctx) {
	tree_turtle_t *stored = array_pop(ctx->stack);

	ctx->turtle = *stored;

	free(stored);
}

tree_generator_t *tree_oak_generator() {
	tree_generator_t *gen = malloc(sizeof(tree_generator_t));

	gen->lsys = l_system_create("BL");

	l_system_add_rule(gen->lsys, "L", "[BL][BL]");

	gen->iterations = 3;
	gen->branch_length = 6.0;
	gen->branch_radius = 2.0;
	gen->size_persistence = 0.7;

	return gen;
}

void tree_generate(world_t *world, tree_generator_t *gen, v3i loc) {
	const char *program = l_system_generate(gen->lsys, gen->iterations);

	tree_turtle_ctx_t ctx = {
		.stack = array_create(0),
		.turtle = (tree_turtle_t){
			(ray_t){
				v3d_add(v3d_from_v3i(loc), BLOCK_CENTER),
				(v3d){0.0, 0.0, gen->branch_length}
			},
			gen->branch_radius
		},
		.angle = rand_angle()
	};

	for (char *c = (char *)program; *c; ++c) {
		switch (*c) {
		case '[':
			tree_turtle_push(&ctx);

			quat_t rotation = tree_turtle_next_rotation(&ctx, 0.125 * M_PI * 2.0);

			ctx.turtle.radius *= gen->size_persistence;
			ctx.turtle.ray.dir = quat_rotate_v3d(ctx.turtle.ray.dir, rotation);

			break;
		case ']':
			tree_turtle_pop(&ctx);
			break;
		case 'B':
			tree_turtle_branch(world, &ctx);

			ctx.turtle.ray.pos = v3d_add(ctx.turtle.ray.pos, ctx.turtle.ray.dir);

			break;
		case 'L':
			tree_turtle_leaves(world, &ctx);
			break;
		}
	}
}
