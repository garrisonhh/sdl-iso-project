#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "world.h"
#include "world/masks.h"
#include "world/bucket.h"
#include "block/block.h"
#include "block/blocks.h"
#include "entity.h"
#include "procgen/noise.h"
#include "player.h"
#include "textures.h"
#include "lib/utils.h"
#include "lib/array.h"
#include "lib/list.h"
#include "pathing.h"

chunk_t *chunk_create() {
	chunk_t *chunk = malloc(sizeof(chunk_t));

	for (int i = 0; i < CHUNK_SIZE; i++) {
		chunk->blocks[i] = NULL;
		chunk->buckets[i] = NULL;
	}

	chunk->num_blocks = 0;
	chunk->num_entities = 0;

	return chunk;
}

void chunk_destroy(chunk_t *chunk) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		if (chunk->blocks[i] != NULL)
			block_destroy(chunk->blocks[i]);
		if (chunk->buckets[i] != NULL)
			list_destroy(chunk->buckets[i], false);
	}

	free(chunk);
}

// sizes are a power of 2
world_t *world_create(unsigned size_power) {
	world_t *world = malloc(sizeof(world_t));

	world->size_power = size_power;
	world->size = 1 << world->size_power;
	world->block_size = world->size << 4;

	world->chunk_mask = world->size - 1;
	world->num_chunks = 1 << (world->size_power * 3);
	world->chunks = malloc(sizeof(chunk_t *) * world->num_chunks);

	for (int i = 0; i < world->num_chunks; i++)
	 	world->chunks[i] = NULL;

	world->mask_updates = array_create(0);
	world->ticks = list_create();
	world->buckets = list_create();
	world->entities = list_create();

	world->path_net = NULL; // set in generate

	return world;
}

void world_destroy(world_t *world) {
	list_node_t *node;

	for (size_t i = 0; i < world->num_chunks; ++i)
		if (world->chunks[i] != NULL)
			chunk_destroy(world->chunks[i]);
	
	LIST_FOREACH(node, world->entities)
		entity_destroy(node->item);

	array_destroy(world->mask_updates, true);
	list_destroy(world->ticks, false);
	list_destroy(world->buckets, false);
	list_destroy(world->entities, false);

	path_network_destroy(world->path_net);

	free(world->chunks);
	free(world);
}

// if you just want to access a block, use world_get() unless you're REALLY going for optimization,
// it is very safe and doesn't have any hidden gotchas
bool world_indices(world_t *world, v3i loc, unsigned *chunk_result, unsigned *block_result) {
	unsigned chunk_index = 0, block_index = 0;
	int dim, i;

	for (i = 0; i < 3; i++) {
		chunk_index <<= world->size_power;
		block_index <<= 4;

		dim = v3i_IDX(loc, i);

		if (dim < 0 || dim >= world->block_size)
			return false;

		chunk_index |= (dim >> 4) & world->chunk_mask;
		block_index |= dim & 0xF;
	}

	*chunk_result = chunk_index;
	*block_result = block_index;

	return true;
}

// call after a potential net decrease in entities/blocks in chunk
void world_check_chunk(world_t *world, unsigned chunk_index) {
	chunk_t *chunk;

	if ((chunk = world->chunks[chunk_index]) != NULL
	 && chunk->num_blocks == 0 && chunk->num_entities == 0) {
		chunk_destroy(chunk);
		world->chunks[chunk_index] = NULL;
	}
}

block_t *world_get(world_t *world, v3i loc) {
	unsigned chunk_index, block_index;
	chunk_t *chunk;

	if (!world_indices(world, loc, &chunk_index, &block_index)
	 || (chunk = world->chunks[chunk_index]) == NULL)
		return NULL;
	return chunk->blocks[block_index];
}

void world_get_render_loc(world_t *world, v3i loc, block_t **block_result, list_t **bucket_result) {
	unsigned chunk_index, block_index;
	chunk_t *chunk;

	if (!world_indices(world, loc, &chunk_index, &block_index)
	 || (chunk = world->chunks[chunk_index]) == NULL) {
		*block_result = NULL;
		*bucket_result = NULL;
	} else {
		*block_result = chunk->blocks[block_index];
		*bucket_result = chunk->buckets[block_index];
	}
}

void world_push_update(world_t *world, v3i loc) {
	v3i *update = malloc(sizeof loc);

	*update = loc;

	array_push(world->mask_updates, update);
}

// only for internal use
void world_set_no_update(world_t *world, v3i loc, size_t block_id) {
	unsigned chunk_index, block_index;
	
	if (!world_indices(world, loc, &chunk_index, &block_index))
		return;

	chunk_t *chunk = world->chunks[chunk_index];

	if (chunk == NULL) {
		chunk = chunk_create();
		world->chunks[chunk_index] = chunk;
	}

	if (chunk->blocks[block_index] != NULL) {
		list_remove(world->ticks, chunk->blocks[block_index]);
		block_destroy(chunk->blocks[block_index]);
	} else {
		++chunk->num_blocks;
	}

	block_t *block = block_create(block_id);

	chunk->blocks[block_index] = block;

	// TODO scalable solution; this checks tickability
	if (block->type == BLOCK_PLANT) {
		list_append(world->ticks, block);
	}
}

void world_set(world_t *world, v3i loc, size_t block_id) {
	v3i offset;

	world_set_no_update(world, loc, block_id);

	FOR_CUBE(offset.x, offset.y, offset.z, -1, 2) {
		world_push_update(world, v3i_add(loc, offset));
	}
}

void world_spawn(world_t *world, entity_t *entity, v3d pos) {
	entity->ray.pos = pos;
	world_bucket_add(world, v3i_from_v3d(entity->ray.pos), entity);
	list_push(world->entities, entity);
}

// TODO better tree generation
void generate_tree(world_t *world, v3i loc) {
	size_t trunk, leaves;
	int max_v, x, y, i;
	double radius;
	v3i leaf_loc;

	trunk = block_gen_get_id("tree trunk");
	leaves = block_gen_get_id("leaves");

	max_v = 3 + rand() % 2;

	for (i = 0; i < max_v + 5; i++) {
		if (i < max_v)
			world_set_no_update(world, loc, trunk);

		radius = 2.5;
		radius = i > max_v ? MIN((5 + max_v) - i, radius) : radius;

		if (i > 3) {
			for (y = loc.y - radius; y <= loc.y + radius; y++) {
				for (x = loc.x - radius; x <= loc.x + radius; x++) {
					if (i < max_v && x == loc.x && y == loc.y)
						continue;
					
					if (sqrt(pow((double)(x - loc.x), 2.0) + pow((double)(y - loc.y), 2.0)) <= radius) {
						leaf_loc = (v3i){x, y, loc.z};
						world_set_no_update(world, leaf_loc, leaves);
					}
				}
			}
		}

		loc.z++;
	}
}

void world_generate(world_t *world) {
	srand(time(0));

	timeit_start();

	if (0) { // debug world
		size_t grass = block_gen_get_id("grass");
		v3i loc;
		/*
		noise3_t *noise;
		double v, scale;

		noise = noise3_create(world->block_size, 1, 1, 0.35);

		FOR_CUBE(loc.x, loc.y, loc.z, 0, world->block_size) {
			v = noise3_at(noise, loc.x, loc.y, loc.z);
			v = 1.0 - fabs(v);

			scale = 4.0 *  (1.0 - 2.0 * ((double)loc.z / (double)(world->block_size - 1)));

			if (v * scale >= 1.0)
				world_set_no_update(world, loc, grass);
		}

		noise3_destroy(noise);
		*/

		loc = (v3i){0, 0, 0};

		FOR_XY(loc.y, loc.x, world->block_size, world->block_size) {
			v3i_print(NULL, loc);
			world_set_no_update(world, loc, grass);
		}
	} else {
		v3i loc;
		double noise_val;
		noise2_t *noise = noise2_create(world->block_size, MAX(world->size_power - 1, 0), 5, 0.5);

		size_t dirt = block_gen_get_id("dirt");
		size_t grass = block_gen_get_id("grass");
		size_t bush = block_gen_get_id("bush");
		size_t tall_grass = block_gen_get_id("tall grass");
		size_t flower = block_gen_get_id("flower");
		size_t sml_rock = block_gen_get_id("small rock");
		// larger rocks don't look good in forest umgebung

		FOR_XY(loc.x, loc.y, world->block_size, world->block_size) {
			noise_val = pow((1.0 + noise2_at(noise, loc.x, loc.y)) / 2, 3.0);
			noise_val *= MIN(world->block_size, 64);

			for (loc.z = 0; loc.z < (int)noise_val; loc.z++)
				world_set_no_update(world, loc, dirt);
			
			world_set_no_update(world, loc, grass);

			++loc.z;

			switch (rand() % 30) {
			case 0:
				world_set_no_update(world, loc, bush);
				break;
			case 1:
			case 2:
			case 3:
				world_set_no_update(world, loc, tall_grass);
				break;
			case 4:
				world_set_no_update(world, loc, flower);
				break;
			case 5:
				world_set_no_update(world, loc, sml_rock);
				break;
			}

			if (rand() % 500 == 0)
				generate_tree(world, loc);
		}

		noise2_destroy(noise);
	}

	timeit_end("world generated");

	v3i loc;

	FOR_CUBE(loc.x, loc.y, loc.z, 0, world->block_size)
		world_update_masks(world, loc);

	timeit_end("block masks updated");

	timeit_start();
	world->path_net = path_generate_world_network(world);
	timeit_end("path network created");
}

void world_tick(world_t *world, double time) {
	size_t i;
	entity_t *entity;
	v3i last_loc, this_loc;
	v3i *update;
	list_node_t *node;

	// update entities and check for bucket swaps
	LIST_FOREACH(node, world->entities) {
		entity = node->item;

		last_loc = v3i_from_v3d(entity->ray.pos);
		entity_tick(entity, world, time);
		this_loc = v3i_from_v3d(entity->ray.pos);

		if (v3i_compare(last_loc, this_loc)) {
			world_bucket_remove(world, last_loc, entity);
			world_bucket_add(world, this_loc, entity);
		}
	}

	// update loops
	for (i = 0; i < world->mask_updates->size; ++i) {
		update = world->mask_updates->items[i];

		world_update_masks(world, *update);
	}

	array_clear(world->mask_updates, true);

	LIST_FOREACH(node, world->ticks)
		block_tick(node->item, world, time);

	LIST_FOREACH(node, world->buckets)
		world_bucket_z_sort(node->item);
}
