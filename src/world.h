#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"
#include "list.h"

#define CHUNK_SIZE 4096 // 0x000 -> 0xFFF
#define GRAVITY (-20)

// TODO typed blocks (some blocks need updates)
struct block_t {
	int texture;
	uint8_t expose_mask;
	// bool expose_update;
};
typedef struct block_t block_t;

struct chunk_t {
	block_t *blocks[CHUNK_SIZE];
	list_t *buckets[CHUNK_SIZE];
};
typedef struct chunk_t chunk_t;

struct world_t {
	chunk_t **chunks;
	unsigned int num_chunks, chunk_mask;
	unsigned int size, size_power;
	unsigned int block_size;
	list_t *entities;
	struct entity_t *player;
};
typedef struct world_t world_t;

world_t *world_create(uint16_t);
void world_destroy(world_t *);
void world_generate(world_t *);
void world_tick(world_t *, int ms);
bool chunk_block_indices(world_t *, v3i, unsigned int *, unsigned int *);
block_t *block_get(world_t *, v3i loc);

#endif
