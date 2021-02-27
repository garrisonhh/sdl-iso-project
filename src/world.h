#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "entity.h"

#define SIZE 16
#define CHUNK_SIZE 4096

typedef struct {
	int texture;
	Uint8 expose_mask;
	bool expose_update;
} block_t;

typedef struct {
	block_t *blocks[CHUNK_SIZE];
	v3i loc;
} chunk_t;

typedef struct {
	chunk_t **chunks;
	v3i dims;
	int num_chunks;
	entity_t *player;
} world_t;

world_t *world_create(v3i dims);
void world_destroy(world_t *);
void world_tick(world_t *, int ms);
void world_generate(world_t *);

#endif
