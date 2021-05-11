#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"
#include "block.h"
#include "data_structures/array.h"
#include "data_structures/list.h"
#include "data_structures/hashmap.h"
#include "pathing.h"

#define CHUNK_SIZE 4096 // 0x000 -> 0xFFF
#define GRAVITY (-20)

struct chunk_t {
	block_t *blocks[CHUNK_SIZE];
	list_t *buckets[CHUNK_SIZE];

	// num_blocks tracked in world_set();
	// num_entities tracked using block bucket funcs
	size_t num_blocks, num_entities;
};
typedef struct chunk_t chunk_t;

struct world_t {
	list_t *mask_updates; // malloc'd v3i *
	list_t *ticks; // block_t *
	list_t *buckets;

	array_t *entities;
	entity_t *player;

	path_network_t *path_net;

	chunk_t **chunks;
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
void world_tick(world_t *, double time);
void world_spawn(world_t *, entity_t *);
block_t *world_get(world_t *, v3i loc);
void world_get_render_loc(world_t *, v3i loc, block_t **block_result, list_t **bucket_result);

#endif
