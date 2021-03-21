#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "utils.h"
#include "world.h"
#include "entity.h"
#include "noise.h"
#include "list.h"
#include "player.h"
#include "textures.h"

block_t *block_create(size_t texture) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	block->texture = texture;
	block->expose_mask = 0x7;

	return block;
}

chunk_t *chunk_create() {
	chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));

	for (int i = 0; i < CHUNK_SIZE; i++) {
		chunk->blocks[i] = NULL;
		chunk->buckets[i] = NULL;
	}

	return chunk;
}

void chunk_destroy(chunk_t *chunk) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		if (chunk->blocks[i] != NULL)
			free(chunk->blocks[i]);
		if (chunk->buckets[i] != NULL)
			list_destroy(chunk->buckets[i]);
	}

	free(chunk);
}

// chunk_t *chunk_get()? maybe completely unnecessary honestly

// returns whether successful (chunk exists), pastes results in pointers if not null
bool chunk_block_indices(world_t *world, v3i loc, unsigned int *chunk_result, unsigned int *block_result) {
	unsigned int chunk_index = 0, block_index = 0;
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

bool block_transparent(block_t *block) {
	return textures[block->texture]->transparent;
}

void block_update_exposure(world_t *world, v3i loc) {
	unsigned int chunk_index, block_index;
	block_t *block, *other_block;
	v3i other_loc;
	bool exposed;

	block = block_get(world, loc);
	exposed = block == NULL || block_transparent(block);

	for (int i = 0; i < 3; i++) {
		other_loc = loc;
		v3i_set(&other_loc, i, v3i_get(&other_loc, i) - 1);

		if (chunk_block_indices(world, other_loc, &chunk_index, &block_index)) {
			other_block = world->chunks[chunk_index]->blocks[block_index];

			if (other_block != NULL) {
				if (block_transparent(other_block))
					continue;

				if (exposed)
					other_block->expose_mask |= 0x04 >> i; // 0b00100
				else
					other_block->expose_mask &= 0x1b >> i; // 0b11011
			}
		}
	}
}

block_t *block_get(world_t *world, v3i loc) {
	unsigned int chunk_index, block_index;

	if (!chunk_block_indices(world, loc, &chunk_index, &block_index))
		return NULL;
	return world->chunks[chunk_index]->blocks[block_index];
}

void block_set(world_t *world, v3i loc, size_t texture) {
	unsigned int chunk_index, block_index;
	
	if (!chunk_block_indices(world, loc, &chunk_index, &block_index))
		return;

	chunk_t *chunk = world->chunks[chunk_index];

	if (chunk->blocks[block_index] != NULL)
		free(chunk->blocks[block_index]);

	chunk->blocks[block_index] = block_create(texture);

	block_update_exposure(world, loc);
}

void block_bucket_add(world_t *world, v3i loc, entity_t *entity) {
	unsigned int chunk_index, block_index;
	
	if (!chunk_block_indices(world, loc, &chunk_index, &block_index)) {
		printf("adding to out of bounds entity bucket.\n");
		exit(1);
	}

	chunk_t *chunk = world->chunks[chunk_index];

	if (chunk->buckets[block_index] == NULL)
		chunk->buckets[block_index] = list_create();
	
	list_add(chunk->buckets[block_index], entity);
}

void block_bucket_remove(world_t *world, v3i loc, entity_t *entity) {
	unsigned int chunk_index, block_index;
	
	if (!chunk_block_indices(world, loc, &chunk_index, &block_index)) {
		printf("removing from out of bounds entity bucket.\n");
		exit(1);
	}

	chunk_t *chunk = world->chunks[chunk_index];

	list_remove(chunk->buckets[block_index], entity);
	
	if (chunk->buckets[block_index]->size == 0) {
		list_destroy(chunk->buckets[block_index]);
		chunk->buckets[block_index] = NULL;
	}
}

// sizes are a power of 2
world_t *world_create(uint16_t size_power) {
	world_t *world = (world_t *)malloc(sizeof(world_t));

	world->size_power = size_power;
	world->size = 1 << world->size_power;
	world->block_size = world->size << 4;
	world->chunk_mask = world->size - 1;
	world->num_chunks = 1 << (world->size_power * 3);
	world->chunks = (chunk_t **)malloc(sizeof(chunk_t *) * world->num_chunks);

	for (int i = 0; i < world->num_chunks; i++)
		world->chunks[i] = chunk_create();

	world->player = player_create();
	world->entities = list_create();

	block_bucket_add(world, v3i_from_v3d(world->player->ray.pos), world->player); 
	list_add(world->entities, world->player);

	return world;
}

void world_destroy(world_t *world) {
	for (int i = 0; i < world->num_chunks; i++)
		chunk_destroy(world->chunks[i]);
	
	list_deep_destroy(world->entities); // includes player

	free(world->chunks);
	free(world);
}

void world_generate(world_t *world) {
	unsigned int x, y, z;
	int noise_val;
	v2d noise_pos;
	v2i dims = {world->size >> 1, world->size >> 1};
	v3i loc;

	size_t dirt, grass, bush, tall_grass;
	dirt = texture_index("dirt");
	grass = texture_index("grass");
	bush = texture_index("bush");
	tall_grass = texture_index("tall_grass");

	srand(time(0));
	noise_init(dims);

	FOR_XY(x, y, world->block_size, world->block_size) {
		loc = (v3i){x, y, 0};
		noise_pos = (v2d){(double)x / 32.0, (double)y / 32.0};
		noise_val = (int)(((1.0 + noise_at(noise_pos)) / 2) * (double)(MIN(world->block_size, 8)));

		for (z = 0; z < noise_val; z++) {
			loc.z = z;
			block_set(world, loc, dirt);
		}
		
		block_set(world, loc, grass);

		loc.z += 1;
		switch (rand() % 20) {
			case 0:
				block_set(world, loc, bush);
				break;
			case 1:
			case 2:
				block_set(world, loc, tall_grass);
				break;
		}
	}

	noise_quit();
}

void world_tick(world_t *world, int ms) {
	// should bucket wrangling happen in entity.c?
	entity_t *entity;
	v3i last_loc, this_loc;

	for (size_t i = 0; i < world->entities->size; i++) {
		entity = world->entities->items[i];

		last_loc = v3i_from_v3d(entity->ray.pos);

		entity_tick(entity, world, ms);

		this_loc = v3i_from_v3d(entity->ray.pos);

		if (v3i_compare(last_loc, this_loc)) {
			block_bucket_remove(world, last_loc, entity);
			block_bucket_add(world, this_loc, entity);
		}
	}
}
