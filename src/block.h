#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include "vector.h"
#include "textures.h"
#include "block_collision.h"
#include "block_types/plant.h"

enum block_type_e {
	BLOCK_STATELESS = 0,
	BLOCK_PLANT = 1,
};
typedef enum block_type_e block_type_e;

union block_state_t {
	plant_t plant;
};
typedef union block_state_t block_state_t;

struct block_t {
	texture_t *texture;
	texture_state_t tex_state;

	block_coll_data_t *coll_data;

	block_type_e type;
	block_state_t state;
};
typedef struct block_t block_t;

block_t *block_create(size_t);
void block_destroy(block_t *);

#endif
