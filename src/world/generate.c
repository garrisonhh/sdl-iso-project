#include <stdlib.h>
#include <time.h>
#include "generate.h"
#include "../world.h"
#include "../block/blocks.h"
#include "../procgen/noise.h"
#include "../lib/utils.h"

void world_gen_flat(world_t *);
void world_gen_alien(world_t *);

void world_generate(world_t *world, world_gen_type_e type) {
	srand(time(0));

	switch (type) {
	case WORLD_FLAT:
		world_gen_flat(world);
		break;
	case WORLD_ALIEN:
		world_gen_alien(world);
		break;
	default:
		break;
	}
}

void world_gen_flat(world_t *world) {
	v3i loc;
	size_t grass = blocks_get_id("grass");

	loc.z = 0;

	FOR_XY(loc.x, loc.y, world->block_size, world->block_size)
		world_set_no_update(world, loc, grass);
}

void world_gen_alien(world_t *world) {
	v3i loc;
	size_t stone = blocks_get_id("stone");
	noise3_t *noise = noise3_create(world->block_size, MAX(world->size_pow2 - 1, 0), 2, 0.25);

	FOR_CUBE(loc.x, loc.y, loc.z, 0, world->block_size) {
		if (noise3_at(noise, loc.x, loc.y, loc.z) > 0) {
			world_set_no_update(world, loc, stone);
		}
	};

	noise3_destroy(noise);
}
