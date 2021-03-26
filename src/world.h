#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"
#include "list.h"

#define CHUNK_SIZE 4096 // 0x000 -> 0xFFF
#define GRAVITY (-20)

/* 
 * TODO texture data, masks etc into a 'tex_data_t' struct for better encapsulation
 */
struct block_t {
	size_t texture;
	uint8_t expose_mask; // last 3 bits are X Y Z
	uint8_t connect_mask; // last 6 bits are +X -X +Y -Y +Z -Z
};
typedef struct block_t block_t;

struct chunk_t {
	block_t *blocks[CHUNK_SIZE];
	list_t *buckets[CHUNK_SIZE];
	size_t num_blocks, num_entities; // TODO remove null chunks?
};
typedef struct chunk_t chunk_t;

struct world_t {
	chunk_t **chunks;
	list_t *entities;
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
