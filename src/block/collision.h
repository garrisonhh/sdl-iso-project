#ifndef BLOCK_COLLISION_H
#define BLOCK_COLLISION_H

#include "../collision.h"
#include <ghh/array.h>

enum block_coll_e {
	BLOCK_COLL_NONE,		// block does not collide
	BLOCK_COLL_DEFAULT_BOX, // block is a normal full block
	BLOCK_COLL_CUSTOM_BOX,  // block uses a custom bbox

	NUM_BLOCK_COLL_TYPES
};
typedef enum block_coll_e block_coll_e;

// used to store collision data for when it's needed
typedef struct block_coll_data {
	block_coll_e coll_type;
	bbox_t *bbox;
} block_coll_data_t;

// used to package collision data for sorting, etc
typedef struct block_collidable {
	v3i loc;
	block_coll_data_t *coll_data;
} block_collidable_t;

void block_coll_array_sort(array_t *, v3d);

#endif
