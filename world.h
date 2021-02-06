#include <stdbool.h>
#include "vector.h"

#ifndef WORLD_H
#define WORLD_H

#define SIZE 16
#define CHUNK_SIZE 4096

typedef struct block {
	int texture;
	unsigned char exposeMask;
} block_t;

// TODO block updates
typedef struct chunk {
	block_t *blocks[CHUNK_SIZE];
	vector3 loc;
} chunk_t;

typedef struct world {
	chunk_t **chunks;
	vector3 dims;
	int numChunks;
} world_t;

world_t *createWorld(vector3 dims);
void destroyWorld(world_t *);
void generateWorld(world_t *);

#endif
