#include <stdlib.h>
#include <time.h>
#include <ghh/utils.h>
#include <ghh/array.h>
#include "generate.h"
#include "gen_tree.h"
#include "../world.h"
#include "../block/blocks.h"
#include "../procgen/noise.h"
#include "../procgen/poisson.h"

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
	double altitude = (-0.5 + ((double)z / (double)side)) * 4.0;

	return v + altitude;
}

void world_gen_normal(world_t *world) {
	BLOCK_DECL(grass);
	BLOCK_DECL(dirt);
	BLOCK_DECL(stone);
	BLOCK_DECL(flower);
	const size_t tall_grass = blocks_get_id("tall grass");

	double v;
	size_t block_id;
	bool place;
	v3i loc, other;
	noise3_t *noise;
	block_t *block;

	noise = noise3_create(world->block_size, MAX(world->size_pow2 - 1, 0), 3, 0.2);

	noise_map_func(noise, world_gen_normal_noise_mapped);

	// generate dirt and stone
	FOR_CUBE(loc.z, loc.y, loc.x, 0, world->block_size) {
		v = noise3_at(noise, loc.x, loc.y, loc.z);

		place = true;

		if (v < 0.0)
			block_id = dirt;
		else if (v < -0.2)
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

	// TODO place plants and trees using poisson distro
	tree_generator_t *oak_gen = tree_oak_generator();
	noise2_t *tree_noise = noise2_create(world->block_size, world->size_pow2, 3, 0.3);
	array_t *tree_samples = poisson2_samples(world->block_size, world->block_size, 24, 30);
	array_t *grass_samples = poisson2_samples(world->block_size, world->block_size, 2, 30);
	v2i sample;

	poisson2_prune_worst(tree_samples, tree_noise, 0.3);
	poisson2_prune_linear(tree_samples, tree_noise, 0.5);
	poisson2_prune_linear(grass_samples, tree_noise, 0.7);

	for (int i = 0; i < array_size(grass_samples); ++i) {
		sample = *(v2i *)array_get(grass_samples, i);

		loc = (v3i){sample.x, sample.y, world->block_size - 1};

		while (world_get(world, loc) == NULL && loc.z >= 0)
			--loc.z;

		++loc.z;
		world_set_no_update(world, loc, ((rand() % 2) ? tall_grass : flower));

		world_get(world, loc)->state.plant.growth = 3.1;
	}

	for (int i = 0; i < array_size(tree_samples); ++i) {
		sample = *(v2i *)array_get(tree_samples, i);

		loc = (v3i){sample.x, sample.y, world->block_size - 1};

		while (world_get(world, loc) == NULL && loc.z > 0)
			--loc.z;

		tree_generate(world, oak_gen, loc);
	}

	noise2_destroy(tree_noise);
	array_destroy(tree_samples, true);
	array_destroy(grass_samples, true);
	noise3_destroy(noise);
}

void world_gen_flat(world_t *world) {
	v3i loc;
	BLOCK_DECL(grass);

	loc.z = 0;

	FOR_XY(loc.x, loc.y, world->block_size, world->block_size)
		world_set_no_update(world, loc, grass);

	// TODO TESTING REMOVE
	/*
	tree_generator_t *oak_gen = tree_oak_generator();

	for (loc.y = 16; loc.y < world->block_size; loc.y += 32)
		for (loc.x = 16; loc.x < world->block_size; loc.x += 32)
			tree_generate(world, oak_gen, loc);
	*/
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
