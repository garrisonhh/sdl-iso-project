#include <stdlib.h>
#include <stdint.h>
#include "block.h"
#include "vector.h"
#include "collision.h"

block_t *block_create(size_t texture, block_coll_type coll_type, bbox_t *bbox, ray_t *plane) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	block->texture = texture;
	block->expose_mask = 0x7;
	block->connect_mask = 0x0;

	block->coll_type = coll_type;
	block->bbox = bbox;
	block->plane = plane;

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}
