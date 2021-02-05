#include <stdlib.h>
#include <stdio.h>
#include "world.h"

/*
in world.*: composite types holding world data and the functions that manage/create/destroy them
*/

// freed in destroyChunk
block_t *createBlock(int texture) {
	block_t *block = (block_t*)malloc(sizeof(block_t));
	block->texture = texture;
	block->exposeMask = 7;
	return block;
}

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

void generateWorld(world_t *world) {
	int x, y, i;
	for (i = 0; i < world->numChunks; i++) {
		for (x = 0; x < SIZE; x++) {
			for (y = 0; y < SIZE; y++) {
				world->chunks[i]->blocks[y * SIZE + x] = createBlock((x + y) % 4 + 1);
			}
		}
	}
}
