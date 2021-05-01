#include <stdlib.h>
#include "block.h"
#include "block_gen.h"
#include "vector.h"

block_state_t *block_state_from_type(block_type_e);

block_t *block_create(size_t block_id) {
	block_t *block = (block_t *)malloc(sizeof(block_t));

	*block = *block_gen_get(block_id);

	block->state = block_state_from_type(block->type);

	return block;
}

void block_destroy(block_t *block) {
	free(block);
}

block_state_t *block_state_from_type(block_type_e type) {
	switch (type) {
		default:
			return NULL;
	}
}
