#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "world.h"

/*
in world.*: composite types holding world data and the functions that manage/create/destroy them
*/

// initializes chunk with no blocks
chunk_t *createChunk(vector3 loc) {
	chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
	if (chunk == NULL) {
		printf("error: could not allocate a new chunk at (%i, %i, %i)\n", loc.x, loc.y, loc.z);
		exit(1);
	}
	for (int i = 0; i < 4096; i++) {
		chunk->blocks[i] = NULL;
	}
	chunk->loc = loc;
	return chunk;
}

void destroyChunk(chunk_t *chunk) {
	for (int i = 0; i < 4096; i++) {
		if (chunk->blocks[i] != NULL) {
			free(chunk->blocks[i]);
			chunk->blocks[i] = NULL;
		}
	}
	free(chunk);
}

void setBlock(chunk_t *chunk, vector3 *loc, int texture) {
	block_t *block = (block_t *)malloc(sizeof(block_t));
	int index = flatten(loc, SIZE);
	if (chunk->blocks[index] != NULL) {
		free(chunk->blocks[index]);
		chunk->blocks[index] = NULL;
	}
	block->texture = texture;
	block->exposeMask = 7;
	chunk->blocks[index] = block;
}

world_t *createWorld(vector3 dims) {
	world_t *world = (world_t *)malloc(sizeof(world_t));
	if (world == NULL) {
		printf("error: could not allocate memory for world.");
		exit(1);
	}
	world->dims = dims;
	world->numChunks = world->dims.x * world->dims.y * world->dims.z;
	world->chunks = (chunk_t **)malloc(sizeof(chunk_t*) * world->numChunks);
	if (world->chunks == NULL) {
		printf("error: could not allocate memory for world chunks.");
		exit(1);
	}
	int x, y, z, index = 0;
	for (z = 0; z < world->dims.z; z++) {
		for (y = 0; y < world->dims.y; y++) {
			for (x = 0; x < world->dims.x; x++) {
				vector3 loc = {x, y, z};
				world->chunks[index++] = createChunk(loc);
			}
		}
	}
	return world;
}

void destroyWorld(world_t *world) {
	for (int i = 0; i < world->numChunks; i++) {
		destroyChunk(world->chunks[i]);
		world->chunks[i] = NULL;
	}
	free(world->chunks);
	free(world);
}

/*
perlin noise:
1) generate random vectors for each point on a grid
	- implementation will randomly choose a slope 1 or -1 diagonal; determined using a 2-bit value at each point
2) for each point within a grid square, calculate the dot product of the vector from the point to each corner and the vector associated with the corner
3) use a linear interpolation formula (cosine based or similar) to interpolate all 4 values
*/
void generateWorld(world_t *world) {
	initNoise(time(0), world->dims.x, world->dims.y);

	dvector2 pos;
	vector3 loc;
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
					val = (int)((1.0 + noise(&pos)) * (SIZE / 2));
					for (cz = 0; cz < SIZE; cz++) {
						loc.z = cz;
						if (cz < val) {
							setBlock(chunk, &loc, (cz % 4) + 1);
						} else {
							break;
						}
					}
				}
			}
		}
	}

	quitNoise();
}
