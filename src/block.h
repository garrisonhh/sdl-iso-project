#ifndef BLOCK_H
#define BLOCK_H

#include <stdlib.h>
#include <stdint.h>
#include "vector.h"
#include "collision.h"

enum block_coll_type {
	BLOCK_COLL_NONE,		// block does not collide
	BLOCK_COLL_DEFAULT_BOX, // block is a normal full block
	BLOCK_COLL_CUSTOM_BOX,  // block uses a custom bbox
	BLOCK_COLL_CHOPPED_BOX, // block is a full block chopped by a plane
};
typedef enum block_coll_type block_coll_type;

/* 
 * TODO texture data, masks etc into a 'tex_data_t' struct for better encapsulation
 */
struct block_t {
	// graphics
	size_t texture;
	uint8_t expose_mask; // last 3 bits are X Y Z
	uint8_t connect_mask; // last 6 bits are +X -X +Y -Y +Z -Z

	// collision; values are relative to box pos
	block_coll_type coll_type;
	bbox_t *bbox;
	ray_t *plane;
};
typedef struct block_t block_t;

block_t *block_create(size_t, block_coll_type, bbox_t *, ray_t *);
void block_destroy(block_t *);

#endif
