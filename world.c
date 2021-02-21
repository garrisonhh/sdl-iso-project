#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "world.h"
#include "noise.h"
#include "entity.h"
#include "player.h"

// initializes chunk with no blocks
chunk_t *createChunk(vector3 loc) {
	chunk_t *chunk = (chunk_t *)malloc(sizeof(chunk_t));
	if (chunk == NULL) {
		printf("error: could not allocate a new chunk at (%i, %i, %i)\n", loc.x, loc.y, loc.z);
		exit(1);
	}
	for (int i = 0; i < CHUNK_SIZE; i++) {
		chunk->blocks[i] = NULL;
	}
	chunk->loc = loc;
	return chunk;
}

void destroyChunk(chunk_t *chunk) {
	for (int i = 0; i < CHUNK_SIZE; i++) {
		if (chunk->blocks[i] != NULL) {
			free(chunk->blocks[i]);
			chunk->blocks[i] = NULL;
		}
	}
	free(chunk);
}

chunk_t *getChunk(world_t *world, vector3 loc) {
	int chunkIndex = ((loc.z * world->dims.z) + loc.y) * world->dims.y + loc.x;
	if (chunkIndex < 0 || chunkIndex >= world->numChunks)
		return NULL;
	return world->chunks[chunkIndex];
}

void setBlock(chunk_t *chunk, vector3 loc, int texture) {
	block_t *block = (block_t *)malloc(sizeof(block_t));
	int index = flatten(loc, SIZE);
	if (chunk->blocks[index] != NULL) {
		free(chunk->blocks[index]);
		chunk->blocks[index] = NULL;
	}
	block->texture = texture;
	block->exposeMask = 7;
	block->updateExpose = true;
	chunk->blocks[index] = block;

	// tell any surrounding blocks to update expose mask on next frame
	for (int offset = 1; offset < CHUNK_SIZE; offset *= SIZE) {
		if (index - offset < 0) {
			break;
		} else if (chunk->blocks[index - offset] != NULL) {
			chunk->blocks[index - offset]->updateExpose = true;
		}
	}
}

block_t *getBlock(world_t *world, vector3 loc) {
	vector3 chunk = {loc.x / SIZE, loc.y / SIZE, loc.z / SIZE};
	
	loc = (vector3){loc.x % SIZE, loc.y % SIZE, loc.z % SIZE};

	return getChunk(world, chunk)->blocks[flatten(loc, SIZE)];
}

void resolveEntityWorldCollision(entity_t *entity, world_t *world) {
	vector3 entityLoc = vector3FromDvector3(entity->pos), loc, chunkLoc, blockLoc;
	chunk_t *chunk;
	bbox_t blockBox = {(dvector3){0, 0, 0}, (dvector3){1, 1, 1}};
	int x, y, z;
	for (z = -1; z <= 1; z++) {
		for (y = -1; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				loc = (vector3){x, y, z};
				loc = vector3Add(entityLoc, loc);
				chunkLoc = (vector3){loc.x / SIZE, loc.y / SIZE, loc.z / SIZE};
				blockLoc = (vector3){loc.x % SIZE, loc.y % SIZE, loc.z % SIZE};
				chunk = getChunk(world, chunkLoc);
				if (chunk != NULL && chunk->blocks[flatten(blockLoc, SIZE)] != NULL) {
					blockBox.offset = dvector3FromVector3(loc);
					resolveEntityCollision(entity, blockBox);
				}
			}
		}
	}
}

void tickWorld(world_t *world, int ms) {
	//world->player->move.z = -1; // TODO proper gravity
	tickEntity(world->player, ms);
	//resolveEntityWorldCollision(world->player, world);
}

world_t *createWorld(vector3 dims) {
	world_t *world = (world_t *)malloc(sizeof(world_t));
	world->dims = dims;
	world->numChunks = world->dims.x * world->dims.y * world->dims.z;
	world->chunks = (chunk_t **)malloc(sizeof(chunk_t*) * world->numChunks);
	
	int x, y, z, index = 0;
	for (z = 0; z < world->dims.z; z++) {
		for (y = 0; y < world->dims.y; y++) {
			for (x = 0; x < world->dims.x; x++) {
				vector3 loc = {x, y, z};
				world->chunks[index++] = createChunk(loc);
			}
		}
	}
	world->player = createPlayer();
	return world;
}

void destroyWorld(world_t *world) {
	for (int i = 0; i < world->numChunks; i++) {
		destroyChunk(world->chunks[i]);
		world->chunks[i] = NULL;
	}
	destroyEntity(world->player);
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
	srand(time(0));
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
					for (cz = 0; cz < val; cz++) {
						loc.z = cz;
						setBlock(chunk, loc, 0); // dirt in ground
					}
					setBlock(chunk, loc, 1); // grass on top
					// randomly scattered bushes
					if ((rand() % 20) == 0) {
						loc.z++;
						setBlock(chunk, loc, 2);
					}
				}
			}
		}
	}

	quitNoise();
}
