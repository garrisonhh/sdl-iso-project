#include <stdlib.h>
#include <time.h>
#include "generate.h"
#include "gen_tree.h"
#include "../world.h"
#include "../block/blocks.h"
#include "../procgen/noise.h"
#include "../lib/utils.h"

void world_gen_normal(world_t *);
void world_gen_flat(world_t *);
void world_gen_alien(world_t *);

void world_generate(world_t *world, world_gen_type_e type) {
	srand(time(0));

	switch (type) {
	case WORLD_NORMAL:
		world_gen_normal(world);
		break;
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

double world_gen_normal_noise_mapped(double v, int side, int x, int y, int z) {
	double altitude = ((double)z / (double)side) * 2.0;

	return (v + 1.0) * altitude;
}

void world_gen_normal(world_t *world) {
	BLOCK_DECL(grass);
	BLOCK_DECL(dirt);
	BLOCK_DECL(stone);

	double v;
	size_t block_id;
	bool place;
	v3i loc, other;
	noise3_t *noise;
	block_t *block;

	noise = noise3_create(world->block_size, world->size_pow2, 5, 0.3);

	noise_map_func(noise, world_gen_normal_noise_mapped);

	// generate dirt and stone
	FOR_CUBE(loc.z, loc.y, loc.x, 0, world->block_size) {
		v = noise3_at(noise, loc.x, loc.y, loc.z);

		place = true;

		if (v < 0.4)
			block_id = dirt;
		else if (v < 0.3)
			block_id = stone;
		else
			place = false;

		if (place)
			world_set_no_update(world, loc, block_id);
	}

	// replace exposed dirt with grass
	FOR_CUBE(loc.z, loc.y, loc.x, 0, world->block_size) {
		if ((block = world_get(world, loc)) != NULL && block->id == dirt) {
			other = loc;
			++other.z;

			if (world_get(world, other) == NULL)
				world_set_no_update(world, loc, grass);
		}
	}

	noise3_destroy(noise);
}

void world_gen_flat(world_t *world) {
	v3i loc;
	BLOCK_DECL(grass);

	loc.z = 0;

	FOR_XY(loc.x, loc.y, world->block_size, world->block_size)
		world_set_no_update(world, loc, grass);

	// TODO TESTING REMOVE
	
	tree_generator_t *oak_gen = tree_oak_generator();

	for (loc.y = 16; loc.y < world->block_size; loc.y += 32) 
		for (loc.x = 16; loc.x < world->block_size; loc.x += 32) 
			tree_generate(world, oak_gen, loc);
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
