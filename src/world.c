#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "utils.h"
#include "world.h"
#include "noise.h"
#include "entity.h"
#include "list.h"
#include "player.h"

const v3d BLOCK_SIZE = {1, 1, 1};

chunk_t *chunk_create(v3i loc) {
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

// loc is a block loc not a chunk loc
chunk_t *get_chunk(world_t *world, v3i loc) {
	int i, coord, chunk_index;
	v3i chunk_loc;

	for (i = 0; i < 3; i++) {
		coord = v3i_get(&loc, i) / SIZE;
		if (coord < 0 || v3i_get(&world->dims, i) <= coord)
			return NULL;
		v3i_set(&chunk_loc, i, coord);
	}

	chunk_index = (chunk_loc.z * world->dims.z + chunk_loc.y) * world->dims.y + chunk_loc.x;

	return world->chunks[chunk_index];
}

// takes an absolute location and returns index within its chunk
int block_index_in_chunk(v3i loc) {
	v3i block_loc;
	int i, coord;

	for (i = 0; i < 3; i++) {
		coord = v3i_get(&loc, i) % SIZE;
		if (coord < 0)
			coord += SIZE;
		v3i_set(&block_loc, i, coord);
	}

	return v3i_flatten(block_loc, SIZE);
}

void set_block(world_t *world, v3i loc, int texture) {
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

block_t *get_block(world_t *world, v3i loc) {
	chunk_t *chunk = get_chunk(world, loc);
	return chunk == NULL ? NULL : chunk->blocks[block_index_in_chunk(loc)];
}

void block_bucket_add(world_t *world, v3i loc, entity_t *entity) {
	int index;
	chunk_t *chunk;

	chunk = get_chunk(world, loc);

	if (chunk == NULL)
		return;

	index = block_index_in_chunk(loc);

	if (chunk->buckets[index] == NULL)
		chunk->buckets[index] = list_create();
	
	list_add(chunk->buckets[index], entity);
}

void block_bucket_remove(world_t *world, v3i loc, entity_t *entity) {
	int index;
	chunk_t *chunk;

	chunk = get_chunk(world, loc);
	
	if (chunk == NULL)
		return;

	index = block_index_in_chunk(loc);
	list_remove(chunk->buckets[index], entity);
	
	if (chunk->buckets[index]->size == 0) {
		list_destroy(chunk->buckets[index]);
		chunk->buckets[index] = NULL;
	}
}

world_t *world_create(v3i dims) {
	int x, y, z, index = 0;
	world_t *world = (world_t *)malloc(sizeof(world_t));
	v3i loc;

	world->dims = dims;
	world->num_chunks = world->dims.x * world->dims.y * world->dims.z;
	world->chunks = (chunk_t **)malloc(sizeof(chunk_t*) * world->num_chunks);

	FOR_XYZ(x, y, z, world->dims.x, world->dims.y, world->dims.z) {
		loc = (v3i){x, y, z};
		world->chunks[index++] = chunk_create(loc);
	}

	world->player = player_create();
	world->entities = list_create();

	block_bucket_add(world, v3i_from_v3d(world->player->ray.pos), world->player); 
	list_add(world->entities, world->player);

	return world;
}

void world_destroy(world_t *world) {
	for (int i = 0; i < world->num_chunks; i++) {
		chunk_destroy(world->chunks[i]);
		world->chunks[i] = NULL;
	}
	entity_destroy(world->player);
	free(world->chunks);
	free(world);
}

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

void world_tick(world_t *world, int ms) {
	// TODO this code needs to go somewhere else
	// find boxes surrounding player
	bbox_t boxes[27];
	int x, y, z, num_boxes = 0;
	v3i player_loc, current_block;

	player_loc = v3i_from_v3d(world->player->ray.pos);

	for (z = -1; z <= 1; z++) {
		for (y = -1; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				current_block = (v3i){x, y, z};
				current_block = v3i_add(player_loc, current_block);
				if (get_block(world, current_block) != NULL)
					boxes[num_boxes++] = (bbox_t){v3d_from_v3i(current_block), BLOCK_SIZE};
			}
		}
	}

	// apply movement + collision; sort ptr into relevant bucket
	int last_bucket, this_bucket;
	v3i this_player_loc;

	world->player->ray.dir.z += GRAVITY * ((double)ms / 1000); // TODO repetition of entity.c, move code

	last_bucket = v3i_flatten(player_loc, SIZE);

	entity_tick(world->player, ms, boxes, num_boxes);

	this_player_loc = v3i_from_v3d(world->player->ray.pos);
	this_bucket = v3i_flatten(this_player_loc, SIZE);

	if (last_bucket != this_bucket) {
		block_bucket_remove(world, player_loc, world->player);
		block_bucket_add(world, this_player_loc, world->player);
	}
}
