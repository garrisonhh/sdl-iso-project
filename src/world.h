#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "world/generate.h"
#include "entity/entity.h"
#include "block/block.h"
#include "lib/vector.h"
#include <ghh/array.h>
#include <ghh/list.h>
#include <ghh/hashmap.h>
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
	array_t *mask_updates; // malloc'd v3i *
	list_t *ticks; // block_t *
	list_t *buckets; // list_t *
	list_t *entities;

	path_network_t *path_net;

	chunk_t **chunks;
	size_t num_chunks;
	unsigned chunk_mask;
	// size = chunks along side of cube
	// block_size = number of blocks along side of cube
	int size, size_pow2, block_size;
};
typedef struct world_t world_t;

chunk_t *chunk_create();
void chunk_destroy();

void world_check_chunk(world_t *, unsigned index);

world_t *world_create(int size_pow2, world_gen_type_e);
void world_destroy(world_t *);

void world_tick(world_t *, double time);

void world_spawn(world_t *, entity_t *, v3d pos);
block_t *world_get(world_t *, v3i loc);
void world_set(world_t *, v3i loc, size_t block_id);
void world_set_no_update(world_t *, v3i loc, size_t block_id);
void world_get_render_loc(world_t *, v3i loc, block_t **block_result, list_t **bucket_result);
bool world_indices(world_t *world, v3i loc, unsigned *chunk_index, unsigned *block_index);

#endif
