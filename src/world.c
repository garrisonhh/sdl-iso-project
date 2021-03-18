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
		if (chunk->blocks[i] != NULL) {
			free(chunk->blocks[i]);
			chunk->blocks[i] = NULL;
		}
		if (chunk->buckets[i] != NULL) {
			free(chunk->buckets[i]);
			chunk->buckets[i] = NULL;
		}
	}
	free(chunk);
}

// bool chunk_get() TODO ?

// returns whether successful (chunk exists), pastes results in pointers if not null
bool chunk_block_indices(world_t *world, v3i loc, unsigned int *chunk_result, unsigned int *block_result) {
	unsigned int chunk_index = 0, block_index = 0;
	int dim, i;

	for (i = 0; i < 3; i++) {
		chunk_index <<= world->size_power;
		block_index <<= 4;

		dim = v3i_get(&loc, i);

		chunk_index |= (dim >> 4) & world->chunk_mask;
		block_index |= dim & 0xF;
	}

	if (chunk_index >= world->num_chunks)
		return false;

	*chunk_result = chunk_index;
	*block_result = block_index;

	return true;
}

block_t *block_get(world_t *world, v3i loc) {
	unsigned int chunk_index, block_index;

	if (!chunk_block_indices(world, loc, &chunk_index, &block_index))
		return NULL;
	return world->chunks[chunk_index]->blocks[block_index];
}

/*
void block_set(world_t *world, v3i loc, int texture) {
	chunk_t *chunk = get_chunk(world, loc);

	if (chunk == NULL)
		return;

	int index = block_index_in_chunk(loc);
	block_t *block = (block_t *)malloc(sizeof(block_t));
	block->texture = texture;
	block->expose_mask = 7;
	block->expose_update = true;

	if (chunk->blocks[index] != NULL)
		free(chunk->blocks[index]);

	chunk->blocks[index] = block;

	// tell any surrounding blocks to update expose mask on next frame
	for (int offset = 1; offset < CHUNK_SIZE; offset *= SIZE) {
		if (index - offset < 0) {
			break;
		} else if (chunk->blocks[index - offset] != NULL) {
			chunk->blocks[index - offset]->expose_update = true;
		}
	}
}
*/

void block_bucket_add(world_t *world, v3i loc, entity_t *entity) {
	unsigned int chunk_index, block_index;
	
	if (!chunk_block_indices(world, loc, &chunk_index, &block_index))
		return;

	chunk_t *chunk = world->chunks[chunk_index];

	if (chunk->buckets[block_index] == NULL)
		chunk->buckets[block_index] = list_create();
	
	list_add(chunk->buckets[block_index], entity);
}

void block_bucket_remove(world_t *world, v3i loc, entity_t *entity) {
	unsigned int chunk_index, block_index;
	
	if (!chunk_block_indices(world, loc, &chunk_index, &block_index))
		return;

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
	world->chunk_mask = world->size - 1;
	world->chunks = (chunk_t **)malloc(sizeof(chunk_t *) * world->size);

	for (int i = 0; i < world->size; i++)
		world->chunks[i] = chunk_create();

	world->player = player_create();
	world->entities = list_create();

	block_bucket_add(world, v3i_from_v3d(world->player->ray.pos), world->player); 
	list_add(world->entities, world->player);

	return world;
}

void world_destroy(world_t *world) {
	for (int i = 0; i < world->size; i++) {
		chunk_destroy(world->chunks[i]);
		world->chunks[i] = NULL;
	}

	entity_destroy(world->player);
	free(world->chunks);
	free(world);
}

/*
// this function is practically legacy code, venture forth if you beware...
void world_generate(world_t *world) {
	v2i dims = {world->dims.x, world->dims.y};
	v2d pos;
	v3i loc;
	int x, y, cx, cy, cz, val;

	srand(time(0));
	noise_init(time(0), dims);

	for (y = 0; y < world->dims.y; y++) {
		for (x = 0; x < world->dims.x; x++) {
			for (cy = 0; cy < SIZE; cy++) {
				for (cx = 0; cx < SIZE; cx++) {
					pos.x = (double)(x * SIZE + cx) / (double)SIZE;
					pos.y = (double)(y * SIZE + cy) / (double)SIZE;
					val = (int)((1.0 + noise_at(pos)) * (SIZE / 4));

					loc.x = x * SIZE + cx;
					loc.y = y * SIZE + cy;

					for (cz = 0; cz < val; cz++) {
						loc.z = cz;
						set_block(world, loc, 0); // dirt in ground
					}

					set_block(world, loc, 1); // grass on top

					// randomly scattered bushes
					if ((rand() % 20) == 0) {
						loc.z++;
						set_block(world, loc, 2);
					}
				}
			}
		}
	}

	noise_quit();
}
*/

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
