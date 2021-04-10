#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"
#include "data_structures/array.h"
#include "data_structures/hashmap.h"
#include "block.h"

#define CHUNK_SIZE 4096 // 0x000 -> 0xFFF
#define GRAVITY (-20)

struct chunk_t {
	block_t *blocks[CHUNK_SIZE];
	array_t *buckets[CHUNK_SIZE]; // TODO make this an array of linked lists, not dynamic arrays

	// num_blocks tracked in block_set();
	// num_entities tracked using block bucket funcs
	size_t num_blocks, num_entities;
};
typedef struct chunk_t chunk_t;

struct world_t {
	chunk_t **chunks;
	array_t *entities;
	struct entity_t *player;

	hashmap_t *NODES; // TODO DEBUG REMOVE

	unsigned num_chunks, chunk_mask;
	// size = chunks along 1 dimension
	// size_power = power of 2 for size
	// block_size = number of blocks along 1 dimension
	unsigned size, size_power, block_size;
};
typedef struct world_t world_t;

world_t *world_create(unsigned);
void world_destroy(world_t *);
void world_generate(world_t *);
void world_tick(world_t *, int ms);
bool chunk_block_indices(world_t *, v3i, unsigned *, unsigned *);
block_t *block_get(world_t *, v3i loc);

#endif
