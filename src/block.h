#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include <stdint.h>
#include "vector.h"
#include "collision.h"
#include "textures.h"
#include "data_structures/array.h"

enum block_coll_e {
	BLOCK_COLL_NONE,		// block does not collide
	BLOCK_COLL_DEFAULT_BOX, // block is a normal full block
	BLOCK_COLL_CUSTOM_BOX,  // block uses a custom bbox
	BLOCK_COLL_CHOPPED_BOX, // block is a full block chopped by a plane
};
typedef enum block_coll_e block_coll_e;

// used to store collision data for when it's needed
struct block_coll_data_t {
	block_coll_e coll_type;
	bbox_t *bbox;
	ray_t *plane;
};
typedef struct block_coll_data_t block_coll_data_t;

// used to package collision data for sorting, etc
struct block_collidable_t {
	v3i loc;
	block_coll_data_t *coll_data;
};
typedef struct block_collidable_t block_collidable_t;

struct block_t {
	texture_t *texture;
	texture_state_t tex_state;

	block_coll_data_t *coll_data;
};
typedef struct block_t block_t;

block_t *block_create(size_t);
void block_destroy(block_t *);
void block_coll_array_sort(array_t *, v3d);

#endif
