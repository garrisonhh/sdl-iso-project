#include <stdlib.h>
#include <stdint.h>
#include "block.h"
#include "block_gen.h"
#include "vector.h"
#include "collision.h"

block_t *block_create(size_t block_id) {
	block_t *block, *model;

	block = (block_t *)malloc(sizeof(block_t));
	model = block_gen_get(block_id);

	// texture
	block->texture = model->texture;
	block->expose_mask = 0x7;
	block->connect_mask = 0x0;

	// collision
	block->coll_type = model->coll_type;

	if (model->bbox != NULL) {
		block->bbox = (bbox_t *)malloc(sizeof(bbox_t));
		*block->bbox = *model->bbox;
	} else {
		block->bbox = NULL;
	}

	if (model->plane != NULL) {
		block->plane = (ray_t *)malloc(sizeof(ray_t));
		*block->plane = *model->plane;
	} else {
		block->plane = NULL;
	}

	return block;
}

void block_destroy(block_t *block) {
	if (block->bbox != NULL)
		free(block->bbox);
	if (block->plane != NULL)
		free(block->plane);
	free(block);
}
