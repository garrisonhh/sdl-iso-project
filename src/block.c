#include <stdlib.h>
#include <stdint.h>
#include "block.h"
#include "block_gen.h"
#include "vector.h"
#include "collision.h"
#include "data_structures/dyn_array.h"

v3i COLL_SORT_POLARITY = {1, 1, 1};

block_t *block_create(size_t block_id) {
	block_t *block, *model;

	block = (block_t *)malloc(sizeof(block_t));
	block->id = block_id;
	model = block_gen_get(block->id);

	// texture
	block->texture = model->texture;
	block->expose_mask = 0x7;
	block->connect_mask = 0x0;

	// collision
	block->coll_data = model->coll_data;

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}

int block_coll_compare(const void *a, const void *b) {
	v3i *this, *other;
	int i, diff;

	this = &(**(block_collidable_t **)a).loc;
	other = &(**(block_collidable_t **)b).loc;

	for (i = 0; i < 3; i++) {
		diff = v3i_get(this, i) - v3i_get(other, i);
		
		if (v3i_get(&COLL_SORT_POLARITY, i) < 0)
			diff = -diff;

		if (diff != 0)
			return diff;
	}

	return 1;
}

void block_coll_dyn_array_sort(dyn_array_t *dyn_array, v3d entity_dir) {
	COLL_SORT_POLARITY = polarity_of_v3d(entity_dir);
	dyn_array_qsort(dyn_array, block_coll_compare);
}

