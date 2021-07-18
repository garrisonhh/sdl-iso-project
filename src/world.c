#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <ghh/utils.h>
#include <ghh/array.h>
#include <ghh/list.h>
#include "world.h"
#include "world/masks.h"
#include "world/bucket.h"
#include "world/generate.h"
#include "block/block.h"
#include "block/blocks.h"
#include "entity/entity.h"
#include "procgen/noise.h"
#include "player.h"
#include "textures.h"
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
world_t *world_create(int size_pow2, world_gen_type_e world_type) {
	v3i loc;

	// fill in obj
	world_t *world = malloc(sizeof(world_t));

	world->size_pow2 = size_pow2;
	world->size = 1 << world->size_pow2;
	world->block_size = world->size << 4;

	world->chunk_mask = world->size - 1;
	world->num_chunks = 1 << (world->size_pow2 * 3);
	world->chunks = malloc(sizeof(chunk_t *) * world->num_chunks);

	for (int i = 0; i < world->num_chunks; i++)
	 	world->chunks[i] = NULL;

	world->mask_updates = array_create(0);
	world->ticks = list_create();
	world->buckets = list_create();
	world->entities = list_create();

	// generate
	world_generate(world, world_type);

	FOR_CUBE(loc.x, loc.y, loc.z, 0, world->block_size)
		world_update_masks(world, loc);

	world->path_net = NULL;
	//world->path_net = path_generate_world_network(world);

	return world;
}

void world_destroy(world_t *world) {
	entity_t *entity;

	for (size_t i = 0; i < world->num_chunks; ++i)
		if (world->chunks[i] != NULL)
			chunk_destroy(world->chunks[i]);

	LIST_FOREACH(entity, world->entities)
		entity_destroy(entity);

	array_destroy(world->mask_updates, true);
	list_destroy(world->ticks, false);
	list_destroy(world->buckets, false);
	list_destroy(world->entities, false);

	//path_network_destroy(world->path_net);

	free(world->chunks);
	free(world);
}

// if you just want to access a block, use world_get() unless you're REALLY going for optimization
bool world_indices(world_t *world, v3i loc, unsigned *out_chunk, unsigned *out_block) {
	*out_chunk = *out_block = 0;

	for (int i = 0; i < 3; i++) {
		*out_chunk <<= world->size_pow2;
		*out_block <<= 4;

		*out_chunk |= (v3i_IDX(loc, i) >> 4) & world->chunk_mask;
		*out_block |= v3i_IDX(loc, i) & 0xF;
	}

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
	if (block->type == BLOCK_PLANT)
		list_append(world->ticks, block);
}

void world_set(world_t *world, v3i loc, size_t block_id) {
	v3i offset;
	v3i *update;

	world_set_no_update(world, loc, block_id);

	FOR_CUBE(offset.x, offset.y, offset.z, -1, 2) {
		update = malloc(sizeof loc);
		*update = v3i_add(loc, offset);
		array_push(world->mask_updates, update);
	}
}

void world_spawn(world_t *world, entity_t *entity, v3d pos) {
	entity->data.ray.pos = pos;
	world_bucket_add(world, v3i_from_v3d(entity->data.ray.pos), entity);
	list_push(world->entities, entity);
}

void world_tick(world_t *world, double time) {
	size_t i;
	v3i last_loc, this_loc;
	v3i *update;
	entity_t *entity;
	block_t *block;
	list_t *bucket;

	// update entities and check for bucket swaps
	LIST_FOREACH(entity, world->entities) {
		last_loc = v3i_from_v3d(entity->data.ray.pos);
		entity_tick(entity, world, time);
		this_loc = v3i_from_v3d(entity->data.ray.pos);

		if (v3i_compare(last_loc, this_loc)) {
			world_bucket_remove(world, last_loc, entity);
			world_bucket_add(world, this_loc, entity);
		}
	}

	// update loops
	for (i = 0; i < array_size(world->mask_updates); ++i) {
		update = array_get(world->mask_updates, i);

		world_update_masks(world, *update);
	}

	array_clear(world->mask_updates, true);

	LIST_FOREACH(block, world->ticks)
		block_tick(block, world, time);

	LIST_FOREACH(bucket, world->buckets)
		world_bucket_z_sort(bucket);
}
