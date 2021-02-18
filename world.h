#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"

#define SIZE 16
#define CHUNK_SIZE 4096

typedef struct {
	int texture;
	Uint8 exposeMask;
	bool updateExpose;
} block_t;

typedef struct {
	block_t *blocks[CHUNK_SIZE];
	vector3 loc;
} chunk_t;

typedef struct {
	chunk_t **chunks;
	vector3 dims;
	int numChunks;
	entity_t *player;
} world_t;

world_t *createWorld(vector3 dims);
void destroyWorld(world_t *);
void generateWorld(world_t *);

#endif
