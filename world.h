#include <stdbool.h>
#include <stdint.h>
#include "vector.h"

#ifndef WORLD_H
#define WORLD_H

#define SIZE 16
#define CHUNK_SIZE 4096

typedef struct {
	int texture;
	Uint8 exposeMask;
} block_t;

typedef struct {
	block_t *blocks[CHUNK_SIZE];
	vector3 loc;
} chunk_t;

typedef struct {
	chunk_t **chunks;
	vector3 dims;
	int numChunks;
} world_t;

world_t *createWorld(vector3 dims);
void destroyWorld(world_t *);
void generateWorld(world_t *);

#endif
