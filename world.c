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

// this also creates the block.. separate functionality maybe? use block ids instead of tex ids? idk
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

block_t *getBlock(chunk_t *chunk, vector3 loc) {
	int blockIndex = flatten(loc, SIZE);

	if (blockIndex >= 0 && blockIndex < CHUNK_SIZE)
		return chunk->blocks[blockIndex];
	return NULL;
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

// TODO collision for entities larger than 1x1x1 (just some annoying math stuff)
void applyEntityCollision(entity_t *entity, world_t *world) {
	int x, y, z, chunkIndex;
	vector3 eLoc = vector3FromDvector3(dvector3Add(entity->pos, entity->bbox.offset)), absLoc, blockLoc;
	dvector3 resolve;
	bbox_t boxArr[27];
	int lenArr = 0; 

	for (z = -1; z <= 1; z++) {
		for (y = -1; y <= 1; y++) {
			for (x = -1; x <= 1; x++) {
				absLoc = (vector3){x, y, z};
				absLoc = vector3Add(eLoc, absLoc);
				chunkIndex = ((absLoc.z / SIZE) * world->dims.z + (absLoc.y / SIZE)) * world->dims.y + (absLoc.x / SIZE);
				blockLoc = (vector3){absLoc.x % SIZE, absLoc.y % SIZE, absLoc.z % SIZE};

				if (chunkIndex >= 0 && chunkIndex < world->numChunks
						&& getBlock(world->chunks[chunkIndex], blockLoc) != NULL) {
					boxArr[lenArr] = (bbox_t){dvector3FromVector3(absLoc), (dvector3){1, 1, 1}};
					lenArr++;

					// TODO remove
					if (absLoc.x == 0 && absLoc.y == 0 && absLoc.z == 0) {
						printf("0, 0, 0 bbox:");
						printfBBox(boxArr[lenArr - 1]);
						printf("\n");
					}
				}
			}
		}
	}

	if (lenArr > 0) {
		resolve = collideResolveMultiple(absoluteBBox(entity), boxArr, lenArr);
		entity->pos = dvector3Add(entity->pos, resolve);
	}
}

void tickWorld(world_t *world, int ms) {
	tickEntity(world->player, ms);
	applyEntityCollision(world->player, world);
}

/*
perlin noise:
1) generate random vectors for each point on a grid
	- implementation will randomly choose a slope 1 or -1 diagonal; determined using a 2-bit value at each point
2) for each point within a grid square, calculate the dot product of the vector from the point to each corner and the vector associated with the corner
3) use a linear interpolation formula (cosine based or similar) to interpolate all 4 values
*/
void generateWorld(world_t *world) {
	// TODO v REMOVE
	vector3 testloc = {0, 0, 1};

	for (int y = 0; y < SIZE; y++) {
		testloc.y = y;
		for (int x = 0; x < SIZE; x++) {
			testloc.x = x;
			setBlock(world->chunks[0], testloc, 1);
		}
	}

	return;
	// TODO ^ REMOVE

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
