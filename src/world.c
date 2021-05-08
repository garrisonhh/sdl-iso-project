#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "world.h"
#include "block.h"
#include "block_gen.h"
#include "entity.h"
#include "noise.h"
#include "player.h"
#include "textures.h"
#include "utils.h"
#include "data_structures/array.h"
#include "data_structures/list.h"
#include "pathing.h"

const v3i OUTLINE_EDGE_OFFSETS[4] = {
	(v3i){ 1,  0, 0},
	(v3i){-1,  0, 0},
	(v3i){ 0,  1, 0},
	(v3i){ 0, -1, 0},
};

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

// if you just want to access a block, use world_get() unless you're REALLY going for optimization,
// it is very safe and doesn't have any hidden gotchas
bool world_indices(world_t *world, v3i loc, unsigned *chunk_result, unsigned *block_result) {
	unsigned chunk_index = 0, block_index = 0;
	int dim, i;

	for (i = 0; i < 3; i++) {
		chunk_index <<= world->size_power;
		block_index <<= 4;

		dim = v3i_get(&loc, i);

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

void world_update_masks(world_t *world, v3i loc) {
	block_t *block;

	if ((block = world_get(world, loc)) != NULL) {
		int i;
		unsigned mask;

		// expose mask
		v3i neighbor;

		mask = 0x0;

		for (i = 0; i < 3; ++i) {
			neighbor = loc;
			v3i_set(&neighbor, i, v3i_get(&neighbor, i) + 1);

			if (block_see_through(world_get(world, neighbor)))
				mask |= 0x1 << i;
		}

		block->expose_mask = mask;

		// connect mask
		if (block->texture->type == TEX_CONNECTED) {
			// TODO re-implement this with connect tags idea
			printf("tex_connected unimplemented currently");
			exit(1);
		}

		if (block->texture->type == TEX_VOXEL) {
			// outline mask
			v3i diagonal;
			int swap;

			mask = 0x0;

			// check each of the top edges based on the edge offset
			for (i = 0; i < 4; ++i)
				if (block_see_through(world_get(world, v3i_add(loc, OUTLINE_EDGE_OFFSETS[i]))))
					mask |= 0x1 << i;

			// check corners
			neighbor = (v3i){0, -1, 0};
			diagonal = (v3i){1, -1, 0};

			for (i = 0; i <= 1; ++i) {
				if (!block_see_through(world_get(world, v3i_add(loc, diagonal)))
				 || block_see_through(world_get(world, v3i_add(loc, neighbor)))) {
					mask |= 0x1 << (i + 4);
				}
				
				SWAP(neighbor.x, neighbor.y, swap);
				SWAP(diagonal.x, diagonal.y, swap);
			}
			
			// check bottom edges
			diagonal = (v3i){1, 0, -1};

			for (i = 0; i <= 1; ++i) {
				if (!block_see_through(world_get(world, v3i_add(loc, diagonal))))
					mask |= 0x1 << (i + 6);
				
				SWAP(diagonal.x, diagonal.y, swap);
			}
			
			block->tex_state.outline_mask = mask;
		}
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

void world_push_update(world_t *world, v3i loc) {
	v3i *update = malloc(sizeof loc);

	*update = loc;

	list_push(world->mask_updates, update);
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

	if (chunk->blocks[block_index] != NULL)
		free(chunk->blocks[block_index]);
	else
		chunk->num_blocks++;

	block_t *block = block_create(block_id);

	chunk->blocks[block_index] = block;

	// TODO this conservatively checks tickability
	if (block->type == BLOCK_PLANT) {
		//printf("id: %lu\n", block_id);
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

void world_bucket_add(world_t *world, v3i loc, entity_t *entity) {
	unsigned chunk_index, block_index;
	
	if (!world_indices(world, loc, &chunk_index, &block_index)) {
		printf("adding to out of bounds entity bucket.\n");
		exit(1);
	}

	chunk_t *chunk = world->chunks[chunk_index];

	if (chunk == NULL) {
		chunk = chunk_create();
		world->chunks[chunk_index] = chunk;
	}

	if (chunk->buckets[block_index] == NULL)
		chunk->buckets[block_index] = list_create();
	
	list_push(chunk->buckets[block_index], entity);
	chunk->num_entities++;
}

void world_bucket_remove(world_t *world, v3i loc, entity_t *entity) {
	unsigned chunk_index, block_index;
	
	if (!world_indices(world, loc, &chunk_index, &block_index)) {
		printf("removing from out of bounds entity bucket.\n");
		exit(1);
	}

	chunk_t *chunk = world->chunks[chunk_index]; // will never be null

	list_remove(chunk->buckets[block_index], entity);
	chunk->num_entities--;
	
	if (chunk->buckets[block_index]->size == 0) {
		list_destroy(chunk->buckets[block_index], false);
		chunk->buckets[block_index] = NULL;
	}

	world_check_chunk(world, chunk_index);
}

void world_spawn(world_t *world, entity_t *entity) {
	world_bucket_add(world, v3i_from_v3d(entity->ray.pos), entity);
	array_add(world->entities, entity);
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
	 	world->chunks[i] = NULL; //world->chunks[i] = chunk_create();

	world->mask_updates = list_create();
	world->ticks = list_create();

	world->player = player_create();
	world->entities = array_create(2);

	world_spawn(world, world->player);

	return world;
}

void world_destroy(world_t *world) {
	size_t i;

	for (i = 0; i < world->num_chunks; ++i)
		if (world->chunks[i] != NULL)
			chunk_destroy(world->chunks[i]);
	
	for (i = 0; i < world->entities->size; ++i)
		entity_destroy(world->entities->items[i]);

	list_destroy(world->mask_updates, true);
	list_destroy(world->ticks, false);
	array_destroy(world->entities, false);
	path_network_destroy(world->path_net);

	free(world->chunks);
	free(world);
}

// TODO this is bad code and doesn't produce very interesting results
void generate_tree(world_t *world, v3i loc) {
	size_t log, leaves;
	int max_v, x, y, i;
	double radius;
	v3i leaf_loc;

	log = block_gen_get_id("log");
	leaves = block_gen_get_id("leaves");

	max_v = 3 + rand() % 2;

	for (i = 0; i < max_v + 5; i++) {
		if (i < max_v) {
			world_set(world, loc, log);
		}

		radius = 2.5;
		radius = i > max_v ? MIN((5 + max_v) - i, radius) : radius;

		if (i > 3) {
			for (y = loc.y - radius; y <= loc.y + radius; y++) {
				for (x = loc.x - radius; x <= loc.x + radius; x++) {
					if (i < max_v && x == loc.x && y == loc.y)
						continue;
					
					if (sqrt(pow((double)(x - loc.x), 2.0) + pow((double)(y - loc.y), 2.0)) <= radius) {
						leaf_loc = (v3i){x, y, loc.z};
						world_set(world, leaf_loc, leaves);
					}
				}
			}
		}

		loc.z++;
	}
}

void world_generate(world_t *world) {
	timeit_start();

	if (0) { // debug world
		size_t ramp = block_gen_get_id("ramp");
		v3i loc;

		loc = (v3i){0, 0, 0};

		for (loc.x = 3; loc.x >= 0; --loc.x) {
			for (loc.y = 3; loc.y >= 0; --loc.y) {
				loc.z = 3 - loc.y;
				world_set_no_update(world, loc, ramp);
			}
		}

	} else {
		int noise_val;
		v2d noise_pos;
		v2i dims = {world->size >> 1, world->size >> 1};
		v3i loc;

		size_t dirt = block_gen_get_id("dirt");
		size_t grass = block_gen_get_id("grass");
		size_t bush = block_gen_get_id("bush");
		size_t tall_grass = block_gen_get_id("tall grass");
		size_t flower = block_gen_get_id("flower");
		size_t sml_rock = block_gen_get_id("small rock");
		// larger rocks don't look good in forest umgebung
		//size_t med_rock = block_gen_get_id("medium rock");
		//size_t lrg_rock = block_gen_get_id("large rock");

		srand(time(0));
		noise_init(dims);

		FOR_XY(loc.x, loc.y, world->block_size, world->block_size) {
			noise_pos = (v2d){(double)loc.x / 32.0, (double)loc.y / 32.0};
			noise_val = (int)(pow((1.0 + noise_at(noise_pos)) / 2, 3.0) * (double)(MIN(world->block_size, 64)));

			for (loc.z = 0; loc.z < noise_val; loc.z++)
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
				/*
				case 6:
					world_set_no_update(world, loc, med_rock);
					break;
				case 7:
					world_set_no_update(world, loc, lrg_rock);
					break;
				*/
			}

			if (rand() % 500 == 0)
				generate_tree(world, loc);
		}

		noise_quit();
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
	// update entities and check for bucket swaps
	entity_t *entity;
	v3i last_loc, this_loc;

	for (size_t i = 0; i < world->entities->size; i++) {
		entity = world->entities->items[i];

		last_loc = v3i_from_v3d(entity->ray.pos);
		entity_tick(entity, world, time);
		this_loc = v3i_from_v3d(entity->ray.pos);

		if (v3i_compare(last_loc, this_loc)) {
			world_bucket_remove(world, last_loc, entity);
			world_bucket_add(world, this_loc, entity);
		}
	}

	// block mask updates
	v3i *update;

	while (world->mask_updates->size) {
		update = (v3i *)list_pop(world->mask_updates);	

		world_update_masks(world, *update);

		free(update);
	}

	// block ticks
	list_node_t *node;

	LIST_FOREACH(node, world->ticks) {
		block_tick(node->item, world, time);
	}
}
