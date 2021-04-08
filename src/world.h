#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"
#include "data_structures/array.h"
#include "block.h"

#define CHUNK_SIZE 4096 // 0x000 -> 0xFFF
#define GRAVITY (-20)

struct chunk_t {
	block_t *blocks[CHUNK_SIZE];
	array_t *buckets[CHUNK_SIZE];
	size_t num_blocks, num_entities; // TODO remove null chunks?
};
typedef struct chunk_t chunk_t;

struct world_t {
	chunk_t **chunks;
	array_t *entities;
	struct entity_t *player;

	unsigned int num_chunks, chunk_mask;
	unsigned int size, size_power;
	unsigned int block_size;
};
typedef struct world_t world_t;

world_t *world_create(uint16_t);
void world_destroy(world_t *);
void world_generate(world_t *);
void world_tick(world_t *, int ms);
bool chunk_block_indices(world_t *, v3i, unsigned int *, unsigned int *);
block_t *block_get(world_t *, v3i loc);

#endif
