#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"
#include "list.h"

#define SIZE 16
#define CHUNK_SIZE 4096
#define GRAVITY (-20)

typedef struct {
	int texture;
	Uint8 expose_mask;
	bool expose_update;
} block_t;

typedef struct {
	block_t *blocks[CHUNK_SIZE];
	list_t *buckets[CHUNK_SIZE];
} chunk_t;

typedef struct {
	v3i dims;
	chunk_t **chunks;
	int num_chunks;
	list_t *entities;
	entity_t *player;
} world_t;

world_t *world_create(v3i dims);
void world_destroy(world_t *);
void world_generate(world_t *);
void world_tick(world_t *, int ms);
block_t *get_block(world_t *, v3i loc);

#endif
