#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include <stdint.h>
#include "vector.h"
#include "collision.h"
#include "textures.h"
#include "block_collision.h"

struct block_t {
	texture_t *texture;
	texture_state_t tex_state;

	block_coll_data_t *coll_data;
};
typedef struct block_t block_t;

block_t *block_create(size_t);
void block_destroy(block_t *);

#endif
