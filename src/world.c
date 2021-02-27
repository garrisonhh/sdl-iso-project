#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "world.h"
#include "noise.h"
#include "entity.h"
#include "player.h"

// initializes chunk with no blocks
chunk_t *create_chunk(v3i loc) {
	chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
	for (int i = 0; i < CHUNK_SIZE; i++) {
		chunk->blocks[i] = NULL;
	}
	chunk->loc = loc;
	return chunk;
}

void destroy_chunk(chunk_t *chunk) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		if (chunk->blocks[i] != NULL) {
			free(chunk->blocks[i]);
			chunk->blocks[i] = NULL;
		}
	}
	free(chunk);
}

// this also creates the block.. separate functionality maybe? use block ids instead of tex ids? idk
void set_block(chunk_t *chunk, v3i loc, int texture) {
	block_t *block = (block_t *)malloc(sizeof(block_t));
	int index = v3i_flatten(loc, SIZE);
	if (chunk->blocks[index] != NULL) {
		free(chunk->blocks[index]);
		chunk->blocks[index] = NULL;
	}
	block->texture = texture;
	block->expose_mask = 7;
	block->expose_update = true;
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

block_t *get_block(chunk_t *chunk, v3i loc) {
	int block_index = v3i_flatten(loc, SIZE);

	if (block_index >= 0 && block_index < CHUNK_SIZE)
		return chunk->blocks[block_index];
	return NULL;
}

world_t *world_create(v3i dims) {
	world_t *world = (world_t *)malloc(sizeof(world_t));
	if (world == NULL) {
		printf("error: could not allocate memory for world.");
		exit(1);
	}
	world->dims = dims;
	world->num_chunks = world->dims.x * world->dims.y * world->dims.z;
	world->chunks = (chunk_t **)malloc(sizeof(chunk_t*) * world->num_chunks);
	if (world->chunks == NULL) {
		printf("error: could not allocate memory for world chunks.");
		exit(1);
	}
	int x, y, z, index = 0;
	for (z = 0; z < world->dims.z; z++) {
		for (y = 0; y < world->dims.y; y++) {
			for (x = 0; x < world->dims.x; x++) {
				v3i loc = {x, y, z};
				world->chunks[index++] = create_chunk(loc);
			}
		}
	}
	world->player = player_create();
	return world;
}

void world_destroy(world_t *world) {
	for (int i = 0; i < world->num_chunks; i++) {
		destroy_chunk(world->chunks[i]);
		world->chunks[i] = NULL;
	}
	entity_destroy(world->player);
	free(world->chunks);
	free(world);
}

void world_tick(world_t *world, int ms) {
	entity_tick(world->player, ms);
	// TODO do I need this?
}

/*
perlin noise:
1) generate random vectors for each point on a grid
	- implementation will randomly choose a slope 1 or -1 diagonal; determined using a 2-bit value at each point
2) for each point within a grid square, calculate the dot product of the vector from the point to each corner and the vector associated with the corner
3) use a linear interpolation formula (cosine based or similar) to interpolate all 4 values
*/
void world_generate(world_t *world) {
	srand(time(0));
	noise_init(time(0), world->dims.x, world->dims.y);

	v2d pos;
	v3i loc;
	int x, y, cx, cy, cz, val;
	chunk_t* chunk;
	for (y = 0; y < world->dims.y; y++) {
		for (x = 0; x < world->dims.x; x++) {
			chunk = world->chunks[y * world->dims.x + x];
			for (cy = 0; cy < SIZE; cy++) {
				for (cx = 0; cx < SIZE; cx++) {
					pos.x = (double)(x * SIZE + cx) / (double)SIZE;
					pos.y = (double)(y * SIZE + cy) / (double)SIZE;
					loc.x = cx;
					loc.y = cy;
					val = (int)((1.0 + noise_at(&pos)) * (SIZE / 2));
					for (cz = 0; cz < val; cz++) {
						loc.z = cz;
						set_block(chunk, loc, 0); // dirt in ground
					}
					set_block(chunk, loc, 1); // grass on top
					// randomly scattered bushes
					if ((rand() % 20) == 0) {
						loc.z++;
						set_block(chunk, loc, 2);
					}
				}
			}
		}
	}

	noise_quit();
}