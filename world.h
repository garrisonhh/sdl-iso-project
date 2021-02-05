#include <stdbool.h>
#include "vector.h"

#ifndef WORLD_H
#define WORLD_H

typedef struct block {
	int texture;
	unsigned char exposeMask;
} block_t;

// TODO block updates
typedef struct chunk {
	block_t *blocks[4096];
	vector3 loc;
} chunk_t;

typedef struct world {
	chunk_t *chunks;
	vector3 dims;
} world_t;

chunk_t *randomTestChunk(vector3);
void destroyChunk(chunk_t *);

#endif
