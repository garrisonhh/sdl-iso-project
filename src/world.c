#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "world.h"
#include "noise.h"
#include "entity.h"
#include "player.h"

const v3d BLOCK_SIZE = {1, 1, 1};

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

block_t *get_block(world_t *world, v3i loc) {
	v3i chunk_loc, block_loc;
	int i, coord, chunk_index;

	// get chunk in world
	for (i = 0; i < 3; i++) {
		coord = v3i_get(&loc, i) / SIZE;
		if (coord < 0 || v3i_get(&world->dims, i) <= coord)
			return NULL;
		v3i_set(&chunk_loc, i, coord);
	}

	// get block in chunk
	for (i = 0; i < 3; i++) {
		coord = v3i_get(&loc, i) % SIZE;
		if (coord < 0 || SIZE <= coord)
			return NULL;
		v3i_set(&block_loc, i, coord);
	}

	chunk_index = (chunk_loc.z * world->dims.z + chunk_loc.y) * world->dims.y + chunk_loc.x;

	return world->chunks[chunk_index]->blocks[v3i_flatten(block_loc, SIZE)];
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
	// tick player
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

	entity_tick(world->player, ms, boxes, num_boxes);
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
