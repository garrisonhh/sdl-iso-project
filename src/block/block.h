#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include "../lib/vector.h"
#include "../textures.h"
#include "collision.h"
#include "plant.h"

typedef struct world_t world_t;

enum block_type_e {
	BLOCK_STATELESS,
	BLOCK_PLANT,

	NUM_BLOCK_TYPES
};
typedef enum block_type_e block_type_e;

struct block_state_t {
	int ticks: 1;
	plant_t plant;
};
typedef struct block_state_t block_state_t;

struct block_t {
	texture_t *texture;

	// bits 0-3 are -X +X -Y +Y ; bit 4 is +Z
	unsigned expose_mask: 5;
	texture_state_t tex_state;

	block_coll_data_t *coll_data;

	block_type_e type;
	block_state_t state;
};
typedef struct block_t block_t;

block_t *block_create(size_t);
void block_destroy(block_t *);

void block_tick(block_t *, world_t *, double time);

#endif
