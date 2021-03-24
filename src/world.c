#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "world.h"
#include "entity.h"
#include "noise.h"
#include "list.h"
#include "player.h"
#include "textures.h"
#include "utils.h"

block_t *block_create(size_t texture) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	block->texture = texture;
	block->expose_mask = 0x7;
	block->connect_mask = 0x0;

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

void block_update_masks(world_t *world, v3i loc) {
	unsigned int chunk_index, block_index;
	block_t *block, *other_block;
	v3i other_loc;
	bool exposed, connected;

	block = block_get(world, loc);
	exposed = block == NULL || block_transparent(block);
	connected = block != NULL && textures[block->texture]->type == TEX_CONNECTED;

	for (int i = 0; i < 3; i++) {
		other_loc = loc;
		v3i_set(&other_loc, i, v3i_get(&other_loc, i) - 1);

		if (chunk_block_indices(world, other_loc, &chunk_index, &block_index)) {
			other_block = world->chunks[chunk_index]->blocks[block_index];

			if (other_block != NULL) {
				if (!block_transparent(other_block)) {
					if (exposed)
						other_block->expose_mask |= 0x04 >> i; // 0b00100
					else
						other_block->expose_mask &= 0x1b >> i; // 0b11011
				}

				if (connected) {
					// 0x20 == 0b00100000
					// 0x10 == 0b00010000
					// 0xEF == !0x10
					if (other_block->texture == block->texture) {
						other_block->connect_mask |= 0x20 >> (i << 1);
						block->connect_mask |= 0x10 >> (i << 1);
					} else {
						block->connect_mask &= 0xFEF >> (i << 1);
					}
				}
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

	block_update_masks(world, loc);
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

void generate_tree(world_t *world, v3i loc) {
	size_t log, leaves;
	int max_v, x, y, i, radius;
	v3i leaf_loc;

	log = texture_index("log");
	leaves = texture_index("leaves");

	max_v = 3 + rand() % 2;

	for (i = 0; i < max_v + 5; i++) {
		if (i < max_v) {
			block_set(world, loc, log);
		}

		radius = 2;
		radius = i > max_v ? MIN((5 + max_v) - i, radius) : radius;

		if (i > 3) {
			for (y = loc.y - radius; y <= loc.y + radius; y++) {
				for (x = loc.x - radius; x <= loc.x + radius; x++) {
					if (i < max_v && x == loc.x && y == loc.y)
						continue;
					
					if (sqrt(pow((double)(x - loc.x), 2.0) + pow((double)(y - loc.y), 2.0)) <= radius) {
						leaf_loc = (v3i){x, y, loc.z};
						block_set(world, leaf_loc, leaves);
					}
				}
			}
		}

		loc.z++;
	}
}

void world_generate(world_t *world) {
	if (0) { // debug world
		int x, y, z;
		size_t dirt = texture_index("dirt");
		v3i loc = {0, 0, 0};

		srand(time(0));

		FOR_XYZ(x, y, z, world->block_size, world->block_size, 5) {
			loc.x = x;
			loc.y = y;
			loc.z = z;

			block_set(world, loc, dirt);

			if ((rand() % 500) == 0)
				generate_tree(world, loc);
		}

		return;
	}

	unsigned int x, y, z;
	int noise_val;
	v2d noise_pos;
	v2i dims = {world->size >> 1, world->size >> 1};
	v3i loc;

	size_t dirt, grass, bush, tall_grass;
	dirt = texture_index("dirt");
	grass = texture_index("grass");
	bush = texture_index("bush");
	tall_grass = texture_index("tall grass");

	srand(time(0));
	noise_init(dims);

	FOR_XY(x, y, world->block_size, world->block_size) {
		loc = (v3i){x, y, 0};
		noise_pos = (v2d){(double)x / 32.0, (double)y / 32.0};
		noise_val = (int)(pow((1.0 + noise_at(noise_pos)) / 2, 3.0) * (double)(MIN(world->block_size, 64)));

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

		if (rand() % 500 == 0)
			generate_tree(world, loc);
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
