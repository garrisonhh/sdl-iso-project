#include <stdlib.h>
#include <stdint.h>
#include "block.h"
#include "block_gen.h"
#include "vector.h"
#include "collision.h"
#include "data_structures/array.h"
#include "textures.h"

v3i COLL_SORT_POLARITY = {1, 1, 1};

block_t *block_create(size_t block_id) {
	block_t *block, *model;

	block = (block_t *)malloc(sizeof(block_t));
	model = block_gen_get(block_id);

	block->texture = model->texture;
	block->tex_state = block_tex_state_from(block->texture->type);

	block->coll_data = model->coll_data;

	// TODO REMOVE
	if (block->texture->type == TEX_SHEET) {
		v2i size = block->texture->tex.sheet->size;
		block->tex_state.state.sheet_cell = (v2i){rand() % size.x, rand() % size.y};
	}

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

void block_coll_array_sort(array_t *array, v3d entity_dir) {
	COLL_SORT_POLARITY = polarity_of_v3d(entity_dir);
	array_qsort(array, block_coll_compare);
}

